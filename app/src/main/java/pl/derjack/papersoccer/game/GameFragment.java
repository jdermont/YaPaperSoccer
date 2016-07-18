package pl.derjack.papersoccer.game;

import android.content.Context;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.support.v7.widget.AppCompatTextView;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;

import java.util.LinkedList;
import java.util.Queue;

import pl.derjack.papersoccer.objects.FieldOptions;
import pl.derjack.papersoccer.objects.FieldView;
import pl.derjack.papersoccer.objects.Game;
import pl.derjack.papersoccer.settings.Settings;

public abstract class GameFragment extends Fragment implements FieldView.FieldViewListener, View.OnClickListener {
    public static final String TAG = "GameFragment";

    protected Settings mSettings;
    protected FieldOptions mFieldOptions;
    protected String mColor1, mColor2;

    protected Game mGame;
    protected int[] mScore;
    protected AppCompatTextView mScoreTxt, mPlayerOneTxt, mPlayerTwoTxt, mTapBallTxt;
    protected FieldView fieldView;

    protected Button mStartGameBtn;
    protected ImageButton mUndoBtn, mUndoBtn1, mUndoBtn2;

    protected boolean mDraftBall;

    protected boolean mIsShown;

    public GameFragment() {
        setRetainInstance(true);
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mSettings = Settings.getInstance(context);
        switch (mSettings.getPlayer1Color()) {
            case "blue": mColor1 = "#0000FF"; break;
            case "green": mColor1 = "#00CC00"; break;
            case "orange": mColor1 = "#CCAA00"; break;
            case "purple": mColor1 = "#FF00FF"; break;
            case "seledine": mColor1 = "#00CCCC"; break;
            case "red":
            default:
                mColor1 = "#FF0000";
        }
        switch (mSettings.getPlayer2Color()) {
            case "red": mColor2 = "#FF0000"; break;
            case "green": mColor2 = "#00CC00"; break;
            case "orange": mColor2 = "#CCAA00"; break;
            case "purple": mColor2 = "#FF00FF"; break;
            case "seledine": mColor2 = "#00CCCC"; break;
            case "blue":
            default:
                mColor2 = "#0000FF";
        }
    }

    public void setFieldOptions(FieldOptions fieldOptions) {
        this.mFieldOptions = fieldOptions;
    }

    protected abstract void updateViews(boolean updateScore);
    protected abstract boolean checkForGameOver();
    protected abstract void changePlayer();
    protected abstract void undo();
    protected abstract void newGame();
    public abstract void onExit();

    protected void makeMove(char m) {
        mGame.makeMove(m);
        fieldView.notifyDraftLinesChanged();
    }

    public void setShown(boolean isShown) {
        this.mIsShown = isShown;
    }
}
