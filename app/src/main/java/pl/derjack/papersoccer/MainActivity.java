package pl.derjack.papersoccer;

import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import pl.derjack.papersoccer.menu.MainFragment;
import pl.derjack.papersoccer_single.R;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (savedInstanceState == null)
        {
            FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
            ft.add(R.id.mainLayout,new MainFragment(), MainFragment.TAG);
            ft.commit();
        }
    }

}
