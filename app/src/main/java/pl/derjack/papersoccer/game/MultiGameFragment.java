package pl.derjack.papersoccer.game;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.Typeface;
import android.os.Bundle;
import android.support.v7.widget.AppCompatTextView;
import android.text.Html;
import android.text.Spanned;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.dialogs.InformationDialog;
import pl.derjack.papersoccer.objects.Cpu;
import pl.derjack.papersoccer.objects.Field;
import pl.derjack.papersoccer.objects.FieldView;
import pl.derjack.papersoccer.objects.Game;

public class MultiGameFragment extends GameFragment {
    public static final String TAG = "MultiGameFragment";

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
            //executor = Executors.newSingleThreadScheduledExecutor();
            //handler = new Handler(this);
            mScore = new int[2];
            mGame = new Game(mFieldOptions);
            mGame.startGame();
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
        View rootView = inflater.inflate(R.layout.fragment_game_multi, container, false);
        mScoreTxt = (AppCompatTextView)rootView.findViewById(R.id.scoreTxt);
        mPlayerOneTxt = (AppCompatTextView)rootView.findViewById(R.id.playerOneTxt);
        mPlayerOneTxt.setText(R.string.guest);
        mPlayerTwoTxt = (AppCompatTextView)rootView.findViewById(R.id.playerTwoTxt);
        mPlayerTwoTxt.setText(mSettings.getPlayerName());
        fieldView = (FieldView)rootView.findViewById(R.id.fieldView);
        fieldView.setColor1(Color.parseColor(mColor1));
        fieldView.setColor2(Color.parseColor(mColor2));
        fieldView.setGame(mGame);
        fieldView.setDraftBall(mDraftBall);
        fieldView.setFieldViewListener(this);
        mTapBallTxt = (AppCompatTextView)rootView.findViewById(R.id.tapBallTxt);
        mStartGameBtn = (Button)rootView.findViewById(R.id.startGameBtn);
        mStartGameBtn.setOnClickListener(this);
        mUndoBtn1 = (ImageButton)rootView.findViewById(R.id.undoBtn1);
        mUndoBtn1.setOnClickListener(this);
        mUndoBtn2 = (ImageButton)rootView.findViewById(R.id.undoBtn2);
        mUndoBtn2.setOnClickListener(this);
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
        mTapBallTxt.setText(mDraftBall ? getString(R.string.tap_the_ball_to_continue) : "");
        if (mGame.currentPlayer == Cpu.Player.ONE) {
            mUndoBtn2.setVisibility(View.INVISIBLE);
            mUndoBtn1.setVisibility((mDraftBall || mGame.canUndo()) ? View.VISIBLE : View.INVISIBLE);
        } else {
            mUndoBtn1.setVisibility(View.INVISIBLE);
            mUndoBtn2.setVisibility((mDraftBall || mGame.canUndo()) ? View.VISIBLE : View.INVISIBLE);
        }
        mStartGameBtn.setVisibility(!mDraftBall && mGame.isOver() ? View.VISIBLE : View.INVISIBLE);
    }

    @Override
    protected boolean checkForGameOver() {
        boolean over = mGame.isOver();
        if (over) {
            if (mGame.getWinner()== Cpu.Player.TWO) mScore[0]++;
            else mScore[1]++;
            InformationDialog dialog = new InformationDialog();
            dialog.setTitle(getString(R.string.game_over));
            dialog.setMessage(mGame.getWinner() == Cpu.Player.TWO ? getString(R.string.you_win) : getString(R.string.you_lose));
            dialog.show(getFragmentManager(), InformationDialog.TAG);
        }
        return over;
    }

    @Override
    protected void changePlayer() {
        mGame.changePlayer();
    }

    @Override
    public void onExit() {

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
        mDraftBall = false;
        fieldView.setDraftBall(mDraftBall);
        fieldView.notifyFieldChanged();
        if (!checkForGameOver()) {
            changePlayer();
        }
        fieldView.invalidate();
        updateViews(true);
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
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.undoBtn1:
            case R.id.undoBtn2:
                undo();
                break;
            case R.id.startGameBtn:
                newGame();
                break;
        }
    }
}
