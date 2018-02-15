package darkbox.heretic.piggyfeeder.home;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import darkbox.heretic.piggyfeeder.R;

/**
 * Created by heretic on 2/10/2018.
 */

public class LightFragment extends Fragment {
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.light_fragment, container, false);
    }
}
