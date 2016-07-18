package pl.derjack.papersoccer.menu;

import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.widget.AppCompatTextView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.dialogs.InformationDialog;
import pl.derjack.papersoccer.game.GameFragment;
import pl.derjack.papersoccer.game.SingleGameFragment;
import pl.derjack.papersoccer.objects.Cpu;
import pl.derjack.papersoccer.objects.Field;
import pl.derjack.papersoccer.objects.FieldOptions;
import pl.derjack.papersoccer.objects.FieldView;
import pl.derjack.papersoccer.objects.Game;

public class TutorialFragment extends GameFragment implements Handler.Callback {
    public static final String TAG = "TutorialFragment";

    private int mCurrentStep;
    private boolean mOver;
    private Handler mHandler;
    private ScheduledExecutorService mExecutor;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);

        if (savedInstanceState == null) {
            mCurrentStep = 0;
            mExecutor = Executors.newSingleThreadScheduledExecutor();
            mHandler = new Handler(this);
            mScore = new int[2];
            mFieldOptions = new FieldOptions(Field.NORMAL,false);
            mGame = new Game(mFieldOptions);
            mGame.startGame();
            showNewDialog();
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        View rootView = inflater.inflate(R.layout.fragment_game_multi, container, false);
        mScoreTxt = (AppCompatTextView)rootView.findViewById(R.id.scoreTxt);
        mScoreTxt.setVisibility(View.INVISIBLE);
        mPlayerOneTxt = (AppCompatTextView)rootView.findViewById(R.id.playerOneTxt);
        mPlayerOneTxt.setText(R.string.guest);
        mPlayerTwoTxt = (AppCompatTextView)rootView.findViewById(R.id.playerTwoTxt);
        mPlayerTwoTxt.setText(mSettings.getPlayerName());
        fieldView = (FieldView)rootView.findViewById(R.id.fieldView);
        fieldView.setColor1(Color.parseColor(mColor1));
        fieldView.setColor2(Color.parseColor(mColor2));
        fieldView.setGame(mGame);
        fieldView.setDraftBall(mDraftBall);
        mTapBallTxt = (AppCompatTextView)rootView.findViewById(R.id.tapBallTxt);
        mTapBallTxt.setText("");
        mStartGameBtn = (Button)rootView.findViewById(R.id.startGameBtn);
        mStartGameBtn.setText(R.string.next_step);
        mStartGameBtn.setOnClickListener(this);
        mUndoBtn1 = (ImageButton)rootView.findViewById(R.id.undoBtn1);
        mUndoBtn1.setVisibility(View.INVISIBLE);
        mUndoBtn2 = (ImageButton)rootView.findViewById(R.id.undoBtn2);
        mUndoBtn2.setVisibility(View.INVISIBLE);
        return rootView;
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();
        updateViews(true);
    }

    @Override
    protected void updateViews(boolean updateScore) {
        if (updateScore) {
            mPlayerOneTxt.setTextColor(Color.parseColor(mColor2));
            mPlayerTwoTxt.setTextColor(Color.parseColor(mColor1));
        }
        if (mGame.currentPlayer == Cpu.Player.TWO) {
            mPlayerTwoTxt.setTypeface(null, Typeface.BOLD);
            mPlayerOneTxt.setTypeface(null, Typeface.NORMAL);
        } else {
            mPlayerTwoTxt.setTypeface(null, Typeface.NORMAL);
            mPlayerOneTxt.setTypeface(null, Typeface.BOLD);
        }
        mStartGameBtn.setVisibility(mOver ? View.VISIBLE: View.INVISIBLE);
        if (mCurrentStep > 4) {
            mStartGameBtn.setText(R.string.exit);
        }
    }

    @Override
    protected boolean checkForGameOver() {
        return mGame.isOver();
    }

    @Override
    protected void changePlayer() {
        mGame.changePlayer();
    }

    @Override
    public void onExit() {
        mExecutor = Executors.newSingleThreadScheduledExecutor();
        mHandler.removeCallbacksAndMessages(null);
    }

    @Override
    protected void newGame() {
        mGame.startGame();
        mDraftBall = false;
        fieldView.notifyDraftLinesChanged();
        fieldView.setNotDrawnLine(null);
        fieldView.notifyFieldChanged();
        fieldView.invalidate();
        updateViews(true);
    }

    @Override
    protected void undo() {
        mGame.undo();
        mDraftBall = false;
        fieldView.notifyDraftLinesChanged();
        fieldView.setDraftBall(mDraftBall);
        fieldView.invalidate();
        updateViews(false);
    }

    @Override
    public void onBallTapped() {

    }

    @Override
    public void onNextPointCalculated(Point pt) {

    }

    @Override
    public void onUp(Point pt) {

    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.startGameBtn:
                if (mCurrentStep <= 4) {
                    mOver = false;
                    showNewDialog();
                    updateViews(false);
                } else {
                    getActivity().finish();
                }
                break;
        }
    }

    private void showNewDialog() {
        String message = "";
        switch (mCurrentStep) {
            case 0: message = getString(R.string.tutorial_0); break;
            case 1: message = getString(R.string.tutorial_1); break;
            case 2: message = getString(R.string.tutorial_2); break;
            case 3: message = getString(R.string.tutorial_3); break;
            case 4: message = getString(R.string.tutorial_4); break;
        }
        InformationDialog dialog = new InformationDialog();
        dialog.setTitle(getString(R.string.tutorial));
        dialog.setMessage(message);
        dialog.setOnClickListener(new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                mExecutor.execute(new Runnable() {
                    @Override
                    public void run() {
                        String move = "";
                        switch (mCurrentStep) {
                            case 0: move = "0"; break;
                            case 1: move = "3"; break;
                            case 2: move = "67"; break;
                            case 3: move = "22,0,2,7,7,17"; break;
                            case 4:
                                mHandler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        newGame();
                                    }
                                });
                                move = "4,6,4,13,3,3,3";
                                break;
                        }
                        char[] moveChars = move.toCharArray();
                        for (int i=0;i<moveChars.length;i++) {
                            if (moveChars[i] != ',') {
                                try {
                                    Thread.sleep(500);
                                } catch (InterruptedException e) {
                                    return;
                                }
                            }
                            Message msg = mHandler.obtainMessage(i==moveChars.length-1?SingleGameFragment.FINISH_MOVE:SingleGameFragment.MAKE_MOVE, moveChars[i]);
                            mHandler.sendMessage(msg);
                        }
                    }
                });
            }
        });
        dialog.show(getFragmentManager(),InformationDialog.TAG);
    }

    @Override
    public boolean handleMessage(Message msg) {
        char m = (char)msg.obj;
        if (m == ',') {
            changePlayer();
            fieldView.notifyFieldChanged();
            updateViews(true);
            fieldView.invalidate();
            return true;
        }
        makeMove(m);
        if (msg.what == SingleGameFragment.FINISH_MOVE) {
            mOver = true;
            mCurrentStep++;
            if (!checkForGameOver()) {
                changePlayer();
            } else {

            }
            fieldView.notifyFieldChanged();
            updateViews(true);
        }
        fieldView.invalidate();
        return true;
    }
}
