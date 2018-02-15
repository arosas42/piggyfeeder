package darkbox.heretic.piggyfeeder.common;

import android.app.Fragment;
import android.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;

/**
 * Created by heretic on 2/10/2018.
 */

public class BaseActivity extends AppCompatActivity {
    static protected Fragment currentFragment = null;

    public void attachFragment(Fragment fragment, int containerID, String tag, boolean backStack) {
        currentFragment = fragment;
        FragmentTransaction ft = getFragmentManager().beginTransaction();

        if(backStack)
            ft.addToBackStack(tag);

        ft.replace(containerID, fragment, tag).commit();
    }
}
