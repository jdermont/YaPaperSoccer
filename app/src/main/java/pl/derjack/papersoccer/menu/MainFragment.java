package pl.derjack.papersoccer.menu;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.FragmentTransaction;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.TutorialActivity;
import pl.derjack.papersoccer.settings.SettingsActivity;
import pl.derjack.papersoccer.settings.SettingsActivityOld;

public class MainFragment extends MenuFragment {
    public static final String TAG = "MainFragment";

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        View rootView = inflater.inflate(R.layout.fragment_main, container, false);
        Button singleBtn = (Button)rootView.findViewById(R.id.singleBtn);
        singleBtn.setOnClickListener(this);
        Button multiBtn = (Button)rootView.findViewById(R.id.multiBtn);
        multiBtn.setOnClickListener(this);
        Button tutorialBtn = (Button)rootView.findViewById(R.id.tutorialBtn);
        tutorialBtn.setOnClickListener(this);
        Button benchmarkBtn = (Button)rootView.findViewById(R.id.benchmarkBtn);
        benchmarkBtn.setOnClickListener(this);
        Button settingsBtn = (Button)rootView.findViewById(R.id.settingsBtn);
        settingsBtn.setOnClickListener(this);
        return rootView;
    }

    @Override
    public void onClick(View v) {
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        switch (v.getId()) {
            case R.id.singleBtn:
                ft.replace(R.id.mainLayout,new SingleFragment(),SingleFragment.TAG);
                ft.addToBackStack(SingleFragment.TAG);
                break;
            case R.id.multiBtn:
                ft.replace(R.id.mainLayout,new MultiFragment(),MultiFragment.TAG);
                ft.addToBackStack(MultiFragment.TAG);
                break;
            case R.id.tutorialBtn: {
                Intent intent = new Intent(getActivity(), TutorialActivity.class);
                startActivity(intent);
                break;
            }
            case R.id.benchmarkBtn: {
                ft.replace(R.id.mainLayout,new BenchmarkFragment(),BenchmarkFragment.TAG);
                ft.addToBackStack(BenchmarkFragment.TAG);
                break;
            }
            case R.id.settingsBtn:
                if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
                    Intent intent = new Intent(getActivity(), SettingsActivity.class);
                    startActivity(intent);
                } else {
                    Intent intent = new Intent(getActivity(), SettingsActivityOld.class);
                    startActivity(intent);
                }
                break;
        }
        ft.commit();
    }
}
