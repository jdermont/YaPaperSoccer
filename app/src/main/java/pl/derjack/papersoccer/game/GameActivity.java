package pl.derjack.papersoccer.game;

import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.objects.Cpu;
import pl.derjack.papersoccer.objects.FieldOptions;

public class GameActivity extends AppCompatActivity {
    public static final String TAG = "GameActivity";

    public static final String GAME_PLAYERS_TYPE = "game_players_type";
    public static final String FIELD_OPTIONS = "field_options";
    public static final String CHOSEN_DIFFICULTY = "chosen_difficulty";

    public static final int SINGLE_PLAYER = 0;
    public static final int MULTI_PLAYER_DEVICE = 1;

    private GameFragment mGameFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_game);
        if (savedInstanceState == null) {
            Intent intent = getIntent();
            mGameFragment = null;
            int playersType = intent.getIntExtra(GAME_PLAYERS_TYPE, SINGLE_PLAYER);
            switch (playersType) {
                case SINGLE_PLAYER:
                    mGameFragment = new SingleGameFragment();
                    int difficulty = intent.getIntExtra(CHOSEN_DIFFICULTY, Cpu.NORMAL);
                    ((SingleGameFragment) mGameFragment).setDifficulty(difficulty);
                    break;
                case MULTI_PLAYER_DEVICE:
                    mGameFragment = new MultiGameFragment();
                    break;
            }
            FieldOptions fieldOptions = intent.getParcelableExtra(FIELD_OPTIONS);
            mGameFragment.setFieldOptions(fieldOptions);
            FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
            ft.add(R.id.gameLayout, mGameFragment, GameFragment.TAG);
            ft.commit();
        } else {
            mGameFragment = (GameFragment)getSupportFragmentManager().findFragmentByTag(GameFragment.TAG);
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();
        mGameFragment.setShown(true);
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause()");
        super.onPause();
        mGameFragment.setShown(false);
    }

    @Override
    public void onBackPressed() {
        Log.d(TAG, "onBackPressed()");
        ExitDialog dialog = new ExitDialog();
        dialog.show(getSupportFragmentManager(), ExitDialog.TAG);
    }

    public void exit() {
        mGameFragment.onExit();
        finish();
    }

    public static class ExitDialog extends DialogFragment {
        public static final String TAG = "ExitDialog";

        public ExitDialog() {
            setRetainInstance(true);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
            alertDialogBuilder.setTitle(getString(R.string.exit));
            alertDialogBuilder.setMessage(getString(R.string.do_you_want_to_exit));
            alertDialogBuilder.setPositiveButton(getString(R.string.yes), new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    ((GameActivity)getActivity()).exit();
                }
            });
            alertDialogBuilder.setNegativeButton(getString(R.string.no), null);
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
