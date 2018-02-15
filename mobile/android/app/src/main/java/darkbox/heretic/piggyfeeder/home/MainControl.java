package darkbox.heretic.piggyfeeder.home;

import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.widget.BottomNavigationView;
import android.util.AttributeSet;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import darkbox.heretic.piggyfeeder.R;
import darkbox.heretic.piggyfeeder.common.BaseActivity;

public class MainControl extends BaseActivity {
    private BottomNavigationView.OnNavigationItemSelectedListener mOnNavigationItemSelectedListener
            = new BottomNavigationView.OnNavigationItemSelectedListener() {

        @Override
        public boolean onNavigationItemSelected(@NonNull MenuItem item) {
            switch (item.getItemId()) {
                case R.id.navigation_status:
                    attachFragment(new StatusFragment(), R.id.main_fragment, "status", false);
                    return true;
                case R.id.navigation_light:
                    attachFragment(new LightFragment(), R.id.main_fragment, "light", false);
                    return true;
                case R.id.navigation_motor:
                    attachFragment(new MotorFragment(), R.id.main_fragment, "motor", false);
                    return true;
            }
            return false;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);
        BottomNavigationView navigation = findViewById(R.id.navigation);
        navigation.setOnNavigationItemSelectedListener(mOnNavigationItemSelectedListener);
        if (currentFragment == null) {
            attachFragment(new StatusFragment(), R.id.main_fragment, "status", false);
        }
    }
}
