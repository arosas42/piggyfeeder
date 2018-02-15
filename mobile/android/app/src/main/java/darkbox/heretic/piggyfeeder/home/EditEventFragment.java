package darkbox.heretic.piggyfeeder.home;

import android.app.Fragment;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import darkbox.heretic.piggyfeeder.R;

/**
 * Created by heretic on 2/11/2018.
 */

public class EditEventFragment extends Fragment implements View.OnClickListener {
    FloatingActionButton mFABCancel;
    FloatingActionButton mFABSave;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.event_fragment, container, false);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mFABCancel = getActivity().findViewById(R.id.cancel_event);
        mFABCancel.setOnClickListener(this);

        mFABSave = getActivity().findViewById(R.id.confirm_event);
        mFABSave.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.cancel_event) {
            getFragmentManager().popBackStack();
        }
    }
}