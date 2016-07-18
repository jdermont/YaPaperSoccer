package pl.derjack.papersoccer.settings;

import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentTransaction;
import android.content.DialogInterface;
import android.os.Build;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import java.util.Arrays;

import pl.derjack.papersoccer_single.R;

public class SettingsActivity extends AppCompatActivity {
    public static final String TAG = "SettingsActivity";

    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        if (savedInstanceState == null) {
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            ft.add(R.id.settingsLayout,new SettingsFragment(), SettingsFragment.TAG);
            ft.commit();
        }
    }

    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class SettingsFragment extends PreferenceFragment {
        public static final String TAG = "SettingsFragment";

        private Settings mSettings;
        private String[] mColors, mColorsValues;
        private Preference mColor1PlayerPreference, mColor2PlayerPreference;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            mSettings = Settings.getInstance(getActivity());
            addPreferencesFromResource(R.xml.settings);

            Preference namePreference = findPreference("player_name");
            namePreference.setSummary(mSettings.getPlayerName());
            namePreference.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                @Override
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    String newName = String.valueOf(newValue);
                    if (newName.trim().isEmpty()) return false;
                    preference.setSummary(newName);
                    return true;
                }
            });

            mColors = getResources().getStringArray(R.array.colors);
            mColorsValues = getResources().getStringArray(R.array.colors_values);
            mColor1PlayerPreference = findPreference("player1_color");
            mColor1PlayerPreference.setSummary(mColors[Arrays.asList(mColorsValues).indexOf(mSettings.getPlayer1Color())]);
            mColor1PlayerPreference.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    ChoiceDialog dialog = new ChoiceDialog();
                    dialog.setTitle(preference.getTitle().toString());
                    dialog.setPosition(Arrays.asList(mColorsValues).indexOf(mSettings.getPlayer1Color()));
                    dialog.setListener(SettingsFragment.this);
                    dialog.setWho(0);
                    dialog.show(getFragmentManager(), ChoiceDialog.TAG);
                    return true;
                }
            });
            mColor2PlayerPreference = findPreference("player2_color");
            mColor2PlayerPreference.setSummary(mColors[Arrays.asList(mColorsValues).indexOf(mSettings.getPlayer2Color())]);
            mColor2PlayerPreference.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    ChoiceDialog dialog = new ChoiceDialog();
                    dialog.setTitle(preference.getTitle().toString());
                    dialog.setPosition(Arrays.asList(mColorsValues).indexOf(mSettings.getPlayer2Color()));
                    dialog.setListener(SettingsFragment.this);
                    dialog.setWho(1);
                    dialog.show(getFragmentManager(), ChoiceDialog.TAG);
                    return true;
                }
            });
        }

        public void onColorChosen(int who, int position) {
            if (who == 0) {
                String color = mColorsValues[position];
                if (color.equals(mSettings.getPlayer2Color())) {
                    mSettings.setPlayer2Color(mSettings.getPlayer1Color());
                    mColor2PlayerPreference.setSummary(mColor1PlayerPreference.getSummary());
                }
                mSettings.setPlayer1Color(color);
                mColor1PlayerPreference.setSummary(mColors[position]);
            } else {
                String color = mColorsValues[position];
                if (color.equals(mSettings.getPlayer1Color())) {
                    mSettings.setPlayer1Color(mSettings.getPlayer2Color());
                    mColor1PlayerPreference.setSummary(mColor1PlayerPreference.getSummary());
                }
                mSettings.setPlayer2Color(color);
                mColor2PlayerPreference.setSummary(mColors[position]);
            }
        }

        public static class ChoiceDialog extends DialogFragment {
            public static final String TAG = "ChoiceDialog";

            private String title = "";
            private int position;
            private SettingsFragment listener;
            private int who;

            public ChoiceDialog() {
                setRetainInstance(true);
            }

            public void setTitle(String title) {
                this.title = title;
            }

            public void setPosition(int position) {
                this.position = position;
            }

            public void setListener(SettingsFragment listener) {
                this.listener = listener;
            }

            public void setWho(int who) {
                this.who = who;
            }

            @Override
            public Dialog onCreateDialog(Bundle savedInstanceState) {
                String[] colors = getResources().getStringArray(R.array.colors);
                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
                alertDialogBuilder.setTitle(title);
                alertDialogBuilder.setSingleChoiceItems(colors, position, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        listener.onColorChosen(who, which);
                        dialog.dismiss();
                    }
                });
                return alertDialogBuilder.create();
            }

            @Override
            public void onDestroyView() {
                if (getDialog() != null && getRetainInstance()) {
                    getDialog().setDismissMessage(null);
                }
                super.onDestroyView();
            }
        }
    }
}
