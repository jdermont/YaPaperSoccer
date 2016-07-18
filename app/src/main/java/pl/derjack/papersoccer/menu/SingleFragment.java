package pl.derjack.papersoccer.menu;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.dialogs.FieldChooserDialog;
import pl.derjack.papersoccer.game.GameActivity;
import pl.derjack.papersoccer.objects.Cpu;
import pl.derjack.papersoccer.objects.FieldOptions;

public class SingleFragment extends MenuFragment {
    public static final String TAG = "SingleFragment";

    private int mChosenDifficulty;

    @Override
    public void onAttach(Context context) {
        Log.d(TAG, "onAttach()");
        super.onAttach(context);
    }

    @Override
    public void onDetach() {
        Log.d(TAG, "onDetach()");
        super.onDetach();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        View rootView = inflater.inflate(R.layout.fragment_menu_single, container, false);
        ((Button)rootView.findViewById(R.id.veryEasyBtn)).setOnClickListener(this);
        ((Button)rootView.findViewById(R.id.easyBtn)).setOnClickListener(this);
        ((Button)rootView.findViewById(R.id.normalBtn)).setOnClickListener(this);
        ((Button)rootView.findViewById(R.id.advancedBtn)).setOnClickListener(this);
        ((Button)rootView.findViewById(R.id.hardBtn)).setOnClickListener(this);
        return rootView;
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause()");
        super.onPause();
    }

    @Override
    public void onStart() {
        Log.d(TAG, "onStart()");
        super.onStart();
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop()");
        super.onStop();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.veryEasyBtn: mChosenDifficulty = Cpu.VERY_EASY; break;
            case R.id.easyBtn: mChosenDifficulty = Cpu.EASY; break;
            case R.id.advancedBtn: mChosenDifficulty = Cpu.ADVANCED; break;
            case R.id.hardBtn: mChosenDifficulty = Cpu.HARD; break;
            default: mChosenDifficulty = Cpu.NORMAL; break;
        }
        FieldChooserDialog dialog = new FieldChooserDialog();
        dialog.setFieldChooserListener(this);
        dialog.show(getFragmentManager(),FieldChooserDialog.TAG);
    }

    @Override
    public void onFieldChosen(FieldOptions fieldOptions) {
        Intent intent = new Intent(getActivity(), GameActivity.class);
        intent.putExtra(GameActivity.GAME_PLAYERS_TYPE, GameActivity.SINGLE_PLAYER);
        intent.putExtra(GameActivity.FIELD_OPTIONS,fieldOptions);
        intent.putExtra(GameActivity.CHOSEN_DIFFICULTY, mChosenDifficulty);
        startActivity(intent);
    }
}
