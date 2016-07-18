package pl.derjack.papersoccer.settings;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.preference.PreferenceManager;

import pl.derjack.papersoccer_single.R;

public class Settings {
    private static Settings sInstance;

    private SharedPreferences mPref;
    private Resources mRes;

    private Settings(Context context) {
        mPref = PreferenceManager.getDefaultSharedPreferences(context.getApplicationContext());
        mRes = context.getApplicationContext().getResources();
    }

    public static synchronized Settings getInstance(Context context) {
        if (sInstance == null) sInstance = new Settings(context);
        return sInstance;
    }

    public String getPlayerName() {
        return mPref.getString("player_name", mRes.getString(R.string.default_player_name)).trim();
    }

    public String getPlayer1Color() {
        return mPref.getString("player1_color","red");
    }

    public void setPlayer1Color(String color) {
        SharedPreferences.Editor editor = mPref.edit();
        editor.putString("player1_color",color);
        editor.apply();
    }

    public String getPlayer2Color() {
        return mPref.getString("player2_color","blue");
    }

    public void setPlayer2Color(String color) {
        SharedPreferences.Editor editor = mPref.edit();
        editor.putString("player2_color",color);
        editor.apply();
    }

    public int getLastChosenFieldSizePosition() {
        return mPref.getInt("last_field_size_position", 1);
    }

    public void setLastChosenFieldSizePosition(int position) {
        SharedPreferences.Editor editor = mPref.edit();
        editor.putInt("last_field_size_position",position);
        editor.apply();
    }

    public boolean getLastChosenHalfLine() {
        return mPref.getBoolean("last_half_line",false);
    }

    public void setLastChosenHalfLine(boolean halfLine) {
        SharedPreferences.Editor editor = mPref.edit();
        editor.putBoolean("last_half_line",halfLine);
        editor.apply();
    }

}
