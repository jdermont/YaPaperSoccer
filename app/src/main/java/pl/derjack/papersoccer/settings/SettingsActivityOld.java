package pl.derjack.papersoccer.settings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;

import java.util.Arrays;

import pl.derjack.papersoccer_single.R;

@SuppressWarnings("deprecation")
public class SettingsActivityOld extends PreferenceActivity {
    private Settings settings;
    private String[] colors,colors_values;
    private Preference color1PlayerPreference,color2PlayerPreference;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        settings = Settings.getInstance(getApplicationContext());

        addPreferencesFromResource(R.xml.settings);

        Preference namePreference = findPreference("player_name");
        namePreference.setSummary(settings.getPlayerName());
        namePreference.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                String newName = String.valueOf(newValue);
                if (newName.trim().isEmpty()) return false;
                preference.setSummary(newName);
                return true;
            }
        });

        colors = getResources().getStringArray(R.array.colors);
        colors_values = getResources().getStringArray(R.array.colors_values);
        color1PlayerPreference = findPreference("player1_color");
        color1PlayerPreference.setSummary(colors[Arrays.asList(colors_values).indexOf(settings.getPlayer1Color())]);
        color1PlayerPreference.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Dialog dialog = getChoiceDialog(preference.getTitle().toString(), Arrays.asList(colors_values).indexOf(settings.getPlayer1Color()),0);
                dialog.show();
                return true;
            }
        });
        color2PlayerPreference = findPreference("player2_color");
        color2PlayerPreference.setSummary(colors[Arrays.asList(colors_values).indexOf(settings.getPlayer2Color())]);
        color2PlayerPreference.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Dialog dialog = getChoiceDialog(preference.getTitle().toString(), Arrays.asList(colors_values).indexOf(settings.getPlayer2Color()),1);
                dialog.show();
                return true;
            }
        });
    }

    public void onColorChosen(int who, int position) {
        if (who == 0) {
            String color = colors_values[position];
            if (color.equals(settings.getPlayer2Color())) {
                settings.setPlayer2Color(settings.getPlayer1Color());
                color2PlayerPreference.setSummary(color1PlayerPreference.getSummary());
            }
            settings.setPlayer1Color(color);
            color1PlayerPreference.setSummary(colors[position]);
        } else {
            String color = colors_values[position];
            if (color.equals(settings.getPlayer1Color())) {
                settings.setPlayer1Color(settings.getPlayer2Color());
                color1PlayerPreference.setSummary(color1PlayerPreference.getSummary());
            }
            settings.setPlayer2Color(color);
            color2PlayerPreference.setSummary(colors[position]);
        }
    }

    public Dialog getChoiceDialog(String title, int position, final int who) {
        String[] colors = getResources().getStringArray(R.array.colors);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(title);
        alertDialogBuilder.setSingleChoiceItems(colors, position, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                onColorChosen(who, which);
                dialog.dismiss();
            }
        });
        return alertDialogBuilder.create();
    }
}
