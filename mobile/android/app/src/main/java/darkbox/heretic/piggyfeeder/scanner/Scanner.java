package darkbox.heretic.piggyfeeder.scanner;

import android.Manifest;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelUuid;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import darkbox.heretic.piggyfeeder.AppState;
import darkbox.heretic.piggyfeeder.R;
import darkbox.heretic.piggyfeeder.common.BaseActivity;

public class Scanner extends ListActivity {
    private BluetoothLeScanner mLEScanner;
    private ScanSettings settings;
    private List<ScanFilter> filters;
    private Handler mHandler;
    private Boolean scanning;
    private ScannerAdapter mDeviceListAdapter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        scanning = false;
        AppState.mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        mHandler = new Handler();
        filters = new ArrayList<>();
        AppState.mFoundDevices = new HashMap<>();
        mDeviceListAdapter = new ScannerAdapter(this, R.layout.device_row);

        setContentView(R.layout.scanner);

        // jesus fuck this language is ugly as shit sometimes.
        ScanFilter sf = new ScanFilter.Builder()
                .setServiceUuid(
                        new ParcelUuid(AppState.PIGGY_SERVICE_UUID),
                        new ParcelUuid(AppState.PIGGY_SERVICE_UUID_MASK)
                ).build();
        filters.add(sf);

        String [] from = new String[] {"name", "addr"};
        int [] to = new int[] {R.id.device_name, R.id.device_addr};
        setListAdapter(mDeviceListAdapter);

        if (AppState.mBluetoothAdapter == null) {
            // the device does not support bluetooth
            // ... wtf am I doing here?
            Toast.makeText(this, "DEVICE DOESNT SUPPORT BLUETOOTH", Toast.LENGTH_SHORT).show();
            finish();
        }

        if (!AppState.mBluetoothAdapter.isEnabled()){
            // need to enable the BT device
            Intent enableBTIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBTIntent, AppState.REQUEST_ENABLE_BT);
        }

        requestPermissions(new String[] {Manifest.permission.ACCESS_COARSE_LOCATION}, AppState.PERMISSION_REQUEST_COARSE_LOCATION);
    }

    protected void startScanning(boolean enable) {

        if (enable) {
            if (scanning)
                return;

            mLEScanner = AppState.mBluetoothAdapter.getBluetoothLeScanner();
            settings = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();

            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mLEScanner.stopScan(mScanCallback);
                    scanning = false;
                }
            }, AppState.SCAN_TIMEOUT);
            mLEScanner.startScan(filters, settings, mScanCallback);
        }
        else {
            scanning = false;
            mLEScanner.stopScan(mScanCallback);
        }
    }

    private ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice btDevice = result.getDevice();
            if (!AppState.mFoundDevices.containsKey(btDevice.getAddress())) {
                AppState.mFoundDevices.put(btDevice.getAddress(), btDevice);
                HashMap<String, String> device = new HashMap<>();
                device.put("name", btDevice.getName());
                device.put("addr", btDevice.getAddress());
                mDeviceListAdapter.add(device);
                mDeviceListAdapter.notifyDataSetChanged();
                Log.i("onScanResult", "Discovered BLE device: " + btDevice.getName() + " (" + btDevice.getAddress() + ")");
            }
        }
    };



    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == AppState.PERMISSION_REQUEST_COARSE_LOCATION && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            startScanning(true);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (AppState.mBluetoothAdapter != null && AppState.mBluetoothAdapter.isEnabled()) {
            startScanning(true);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        startScanning(false);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == AppState.REQUEST_ENABLE_BT && resultCode != RESULT_OK) {
            // uhoh...
            Toast.makeText(this, "UNABLE TO TURN ON BLUETOOTH", Toast.LENGTH_SHORT).show();
            finish();
        }
        else {
            startScanning(true);
        }
    }

    public class ScannerAdapter extends ArrayAdapter<HashMap<String, String>> {
        public ScannerAdapter(@NonNull Context context, int resource) {
            super(context, resource);
        }

        @NonNull
        @Override
        public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
            HashMap<String, String> device = getItem(position);
            if (convertView == null)
                convertView = LayoutInflater.from(getContext()).inflate(R.layout.device_row, parent, false);
            TextView name = convertView.findViewById(R.id.device_name);
            TextView addr = convertView.findViewById(R.id.device_addr);
            name.setText(device.get("name"));
            addr.setText(device.get("addr"));

            return convertView;
        }
    }
}
