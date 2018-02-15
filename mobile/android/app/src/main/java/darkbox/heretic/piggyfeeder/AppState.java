package darkbox.heretic.piggyfeeder;

import android.app.Application;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;

import java.util.HashMap;
import java.util.UUID;

/**
 * Created by heretic on 2/13/2018.
 */

public class AppState extends Application {
   public static BluetoothAdapter mBluetoothAdapter = null;
   public static HashMap<String, BluetoothDevice> mFoundDevices;

   public static final Integer REQUEST_ENABLE_BT = 1;
   public static final Integer PERMISSION_REQUEST_COARSE_LOCATION = 231;
   public static final Integer SCAN_TIMEOUT = 10000;
   public static final UUID PIGGY_SERVICE_UUID = UUID.fromString("0000fff0-0000-1000-8000-00805f9b34fb");
   public static final UUID PIGGY_SERVICE_UUID_MASK = UUID.fromString("0000fff0-0000-0000-0000-000000000000");
}
