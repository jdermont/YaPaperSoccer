package pl.derjack.papersoccer;

import android.os.Bundle;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import pl.derjack.papersoccer.menu.TutorialFragment;
import pl.derjack.papersoccer_single.R;

public class TutorialActivity extends AppCompatActivity {
    public static final String TAG = "TutorialActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_game);

        if (savedInstanceState == null) {
            FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
            ft.add(R.id.gameLayout, new TutorialFragment(), TutorialFragment.TAG);
            ft.commit();
        }
    }
}
