package pl.derjack.papersoccer.game;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.support.v7.widget.AppCompatTextView;
import android.text.Html;
import android.text.Spanned;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.dialogs.InformationDialog;
import pl.derjack.papersoccer.objects.Cpu;
import pl.derjack.papersoccer.objects.Field;
import pl.derjack.papersoccer.objects.FieldView;
import pl.derjack.papersoccer.objects.Game;

public class SingleGameFragment extends GameFragment implements Handler.Callback {
    public static final String TAG = "SingleGameFragment";
    public static final int MAKE_MOVE = 0;
    public static final int FINISH_MOVE = 1;
    private int mDifficulty;
    private long mDelay = 900L;

    private Cpu mCpu;
    private ProgressBar mThinkingIcon;
    private Handler mHandler;
    private ScheduledExecutorService mExecutor;
    private final Runnable mCpuAction = new Runnable() {
        @Override
        public void run() {
            long start = SystemClock.elapsedRealtime();
            String move = mCpu.getMove();
            long stop = SystemClock.elapsedRealtime();
            if (stop-start < mDelay) {
                try {
                    Thread.sleep(mDelay -(stop-start));
                } catch (InterruptedException e) {
                    return;
                }
            }
            if (Thread.currentThread().isInterrupted()) return;
            mCpu.updateGameState(move);
            char[] moveArray = move.toCharArray();
            for (int i=0;i<moveArray.length-1;i++) {
                Message msg = mHandler.obtainMessage(MAKE_MOVE,moveArray[i]);
                mHandler.sendMessage(msg);
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    return;
                }
            }
            Message msg = mHandler.obtainMessage(FINISH_MOVE,moveArray[moveArray.length-1]);
            mHandler.sendMessage(msg);
        }
    };

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

        if (savedInstanceState == null) {
            mExecutor = Executors.newSingleThreadScheduledExecutor();
            mHandler = new Handler(this);
            mScore = new int[2];
            mGame = new Game(mFieldOptions);
            mGame.startGame();
            mCpu = new Cpu(mDifficulty, mFieldOptions);
            mCpu.initialize();
            mCpu.startGame();
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        View rootView = inflater.inflate(R.layout.fragment_game_single, container, false);
        mScoreTxt = (AppCompatTextView)rootView.findViewById(R.id.scoreTxt);
        mPlayerOneTxt = (AppCompatTextView)rootView.findViewById(R.id.cpuTxt);
        String cpuString;
        switch (mDifficulty) {
            case Cpu.VERY_EASY: cpuString = getString(R.string.very_easy); break;
            case Cpu.EASY: cpuString = getString(R.string.easy); break;
            case Cpu.ADVANCED: cpuString = getString(R.string.advanced); break;
            case Cpu.HARD: cpuString = getString(R.string.hard); break;
            default: cpuString = getString(R.string.normal); break;
        }
        mPlayerOneTxt.setText(cpuString);
        mThinkingIcon = (ProgressBar)rootView.findViewById(R.id.thinkingIcon);
        mPlayerTwoTxt = (AppCompatTextView)rootView.findViewById(R.id.playerTxt);
        mPlayerTwoTxt.setText(mSettings.getPlayerName());
        mPlayerTwoTxt.post(new Runnable() {
            @Override
            public void run() {
                RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) mThinkingIcon.getLayoutParams();
                params.width = mPlayerTwoTxt.getHeight();
                params.height = mPlayerTwoTxt.getHeight();
                mThinkingIcon.setLayoutParams(params);
            }
        });
        fieldView = (FieldView)rootView.findViewById(R.id.fieldView);
        fieldView.setColor1(Color.parseColor(mColor1));
        fieldView.setColor2(Color.parseColor(mColor2));
        fieldView.setGame(mGame);
        fieldView.setDraftBall(mDraftBall);
        fieldView.setFieldViewListener(this);
        mTapBallTxt = (AppCompatTextView)rootView.findViewById(R.id.tapBallTxt);
        mStartGameBtn = (Button)rootView.findViewById(R.id.startGameBtn);
        mStartGameBtn.setOnClickListener(this);
        mUndoBtn = (ImageButton)rootView.findViewById(R.id.undoBtn);
        mUndoBtn.setOnClickListener(this);
        return rootView;
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();
        updateViews(true);
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

    public void setDifficulty(int difficulty) {
        this.mDifficulty = difficulty;
        switch (difficulty) {
            case Cpu.VERY_EASY:
            case Cpu.EASY:
                mDelay = 700L;
                break;
            case Cpu.NORMAL:
                mDelay = 900L;
                break;
            case Cpu.ADVANCED:
                mDelay = 1100L;
                break;
            case Cpu.HARD:
                mDelay = 1300L;
        }
    }

    @Override
    public void onBallTapped() {
        mDraftBall = false;
        fieldView.setDraftBall(mDraftBall);
        fieldView.notifyFieldChanged();
        mCpu.updateGameState(mGame.field, mGame.getDraftEdges());
        if (!checkForGameOver()) {
            changePlayer();
        }
        fieldView.invalidate();
        updateViews(true);
    }

    @Override
    protected boolean checkForGameOver() {
        boolean over = mGame.isOver();
        if (over) {
            if (mGame.getWinner()== Cpu.Player.TWO) mScore[0]++;
            else mScore[1]++;
            InformationDialog dialog = new InformationDialog();
            dialog.setTitle(getString(R.string.game_over));
            dialog.setMessage(mGame.getWinner()== Cpu.Player.TWO?getString(R.string.you_win):getString(R.string.you_lose));
            dialog.show(getFragmentManager(), InformationDialog.TAG);
        }
        return over;
    }

    @Override
    protected void changePlayer() {
        mGame.changePlayer();
        if (mGame.currentPlayer == Cpu.Player.ONE) {
            fieldView.setFieldViewListener(null);
            mExecutor.execute(mCpuAction);
        } else {
            fieldView.setFieldViewListener(this);
        }
    }

    @Override
    public void onNextPointCalculated(Point pt) {
        boolean updateFieldView;
        int next = mGame.field.getNeibghour(pt.x, pt.y);
        if (next == -1) {
            updateFieldView = fieldView.setNotDrawnLine(null);
        } else {
            Field.Line line = new Field.Line(mGame.field.getPosition(mGame.field.ball), mGame.field.getPosition(next));
            updateFieldView = fieldView.setNotDrawnLine(line);
        }
        if (updateFieldView) fieldView.invalidate();
    }

    @Override
    public void onUp(Point pt) {
        fieldView.setNotDrawnLine(null);
        if (pt != null) {
            int next = mGame.field.getNeibghour(pt.x, pt.y);
            if (next != -1) {
                mGame.makeMove(next);
                fieldView.notifyDraftLinesChanged();
                if (mGame.toChangeTurn() || mGame.isOver()) {
                    mDraftBall = true;
                    fieldView.setDraftBall(mDraftBall);
                }
            }
        }
        fieldView.invalidate();
        updateViews(false);
    }

    @Override
    protected void updateViews(boolean updateScore) {
        if (updateScore) {
            String scoreString = String.format("<font color=\"%s\">%02d</font> : <font color=\"%s\">%02d</font>",
                    mColor1, mScore[0], mColor2, mScore[1]);
            Spanned scoreSpannedString = Html.fromHtml(scoreString);
            mScoreTxt.setText(scoreSpannedString);
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
        mThinkingIcon.setVisibility(mGame.currentPlayer == Cpu.Player.ONE && !mGame.isOver() ? View.VISIBLE : View.INVISIBLE);
        mTapBallTxt.setText(mDraftBall ? getString(R.string.tap_the_ball_to_continue) : "");
        mUndoBtn.setVisibility((mDraftBall || mGame.canUndo()) ? View.VISIBLE : View.INVISIBLE);
        mStartGameBtn.setVisibility(!mDraftBall && mGame.isOver() ? View.VISIBLE : View.INVISIBLE);
    }

    @Override
    protected void newGame() {
        mGame.startGame();
        mCpu.startGame();
        mDraftBall = false;
        fieldView.notifyDraftLinesChanged();
        fieldView.setNotDrawnLine(null);
        fieldView.notifyFieldChanged();
        fieldView.invalidate();
        updateViews(true);
        if (mGame.currentPlayer == Cpu.Player.ONE) {
            fieldView.setFieldViewListener(null);
            mExecutor.execute(mCpuAction);
        } else {
            fieldView.setFieldViewListener(this);
        }
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
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.undoBtn:
                undo();
                break;
            case R.id.startGameBtn:
                newGame();
                break;
        }
    }

    @Override
    public void onExit() {
        mExecutor.shutdownNow();
        mHandler.removeCallbacksAndMessages(null);
        mCpu.release();
    }

    @Override
    public boolean handleMessage(Message msg) {
        char m = (char)msg.obj;
        makeMove(m);
        if (msg.what == FINISH_MOVE) {
            if (!checkForGameOver()) {
                changePlayer();
            }
            fieldView.notifyFieldChanged();
            updateViews(true);
        }
        fieldView.invalidate();
        return true;
    }
}
