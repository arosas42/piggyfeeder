package darkbox.heretic.piggyfeeder.home;

import android.app.ListFragment;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Toast;

import darkbox.heretic.piggyfeeder.R;
import darkbox.heretic.piggyfeeder.common.BaseActivity;

/**
 * Created by heretic on 2/10/2018.
 */

public class MotorFragment extends ListFragment implements View.OnClickListener, AdapterView.OnItemClickListener {
    Button mRunMotor;
    FloatingActionButton mFAB;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mRunMotor = getActivity().findViewById(R.id.run_motor_button);
        mRunMotor.setOnClickListener(this);

        mFAB = getActivity().findViewById(R.id.add_event);
        mFAB.setOnClickListener(this);

        ArrayAdapter adapter = ArrayAdapter.createFromResource(getActivity(), R.array.Planets, android.R.layout.simple_list_item_1);
        setListAdapter(adapter);
        getListView().setOnItemClickListener(this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.motor_listfragment, container, false);
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.run_motor_button) {
            Toast.makeText(getActivity(), "RUN MOTOR!!!", Toast.LENGTH_SHORT).show();
        }
        else if (view.getId() == R.id.add_event) {
            Toast.makeText(getActivity(), "FAB PRESSED", Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        ((BaseActivity)getActivity()).attachFragment(new EditEventFragment(), R.id.main_fragment, "event", true);
    }
}
