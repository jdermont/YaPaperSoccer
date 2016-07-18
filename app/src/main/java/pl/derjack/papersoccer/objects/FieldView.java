package pl.derjack.papersoccer.objects;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import java.util.ArrayList;
import java.util.List;

public class FieldView extends View {
    public String TAG = "FieldView";

    private AspectRatioClass mAspectRatioClass;

    private FieldViewListener mListener;

    // mGame
    private Game mGame;
    private boolean mDraftBall;

    // colors
    private int mColor1, mColor2;

    // dimensions
    private Paint mPaint;
    private int mLineWidth;
    private int mBoldLineWidth;
    private int mPointRadius;
    private int mBlockSize;

    // cached lines
    private float[] mBackgroundLines;
    private float[] mCachedFieldLines;
    private float[] mCachedFieldLinesOne;
    private float[] mCachedFieldLinesTwo;
    private boolean mFieldChanged;
    private boolean mDraftLinesChanged;
    private float[] mDraftLines;
    private Cpu.Player mDraftPlayer;
    private Field.Line mNotDrawnLine;

    public FieldView(Context context) {
        super(context);
    }

    public FieldView(Context context, AttributeSet attrs) {
        super(context, attrs);

        int dpi = context.getResources().getDisplayMetrics().densityDpi;
        if (dpi <= DisplayMetrics.DENSITY_LOW) {
            mLineWidth = 1;
            mBoldLineWidth = 2;
            mPointRadius = 3;
        } else if (dpi <= DisplayMetrics.DENSITY_MEDIUM) {
            mLineWidth = 2;
            mBoldLineWidth = 3;
            mPointRadius = 4;
        } else if (dpi <= DisplayMetrics.DENSITY_HIGH) {
            mLineWidth = 3;
            mBoldLineWidth = 5;
            mPointRadius = 6;
        } else if (dpi <= DisplayMetrics.DENSITY_XHIGH) {
            mLineWidth = 4;
            mBoldLineWidth = 6;
            mPointRadius = 8;
        } else if (dpi <= DisplayMetrics.DENSITY_XXHIGH) {
            mLineWidth = 6;
            mBoldLineWidth = 9;
            mPointRadius = 12;
        } else {
            mLineWidth = 8;
            mBoldLineWidth = 12;
            mPointRadius = 16;
        }
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setStrokeWidth(mLineWidth);
        mColor1 = Color.RED;
        mColor2 = Color.BLUE;
    }

    public void setFieldViewListener(FieldViewListener listener) {
        this.mListener = listener;
    }

    public void setColor1(int color1) {
        this.mColor1 = color1;
    }

    public void setColor2(int color2) {
        this.mColor2 = color2;
    }

    public void setGame(Game game) {
        this.mGame = game;
        mAspectRatioClass = new AspectRatioClass(1f*(game.field.mWidth +1)/(game.field.mHeight +3));
        mBlockSize = 0;
        clearTemps();
        requestLayout();
    }

    public void setDraftBall(boolean draftBall) {
        this.mDraftBall = draftBall;
    }

    public void notifyFieldChanged() {
        mFieldChanged = true;
    }

    public void notifyDraftLinesChanged() {
        mDraftLinesChanged = true;
    }

    private void clearTemps() {
        mBackgroundLines = null;
        mCachedFieldLines = null;
        mFieldChanged = false;
        mDraftPlayer = Cpu.Player.NONE;
        mDraftLines = null;
        mNotDrawnLine = null;
    }

    public boolean setNotDrawnLine(Field.Line line) {
        if (line == null) {
            boolean update = mNotDrawnLine !=null;
            mNotDrawnLine = null;
            return update;
        }
        line = new Field.Line(line);
        line.a.x = mBlockSize /2 + mBlockSize * line.a.x;
        line.a.y = 3* mBlockSize /2 + mBlockSize * line.a.y;
        line.b.x = mBlockSize /2 + mBlockSize * line.b.x;
        line.b.y = 3* mBlockSize /2 + mBlockSize * line.b.y;
        if (line.equals(mNotDrawnLine)) return false;
        this.mNotDrawnLine = line;
        return true;
    }

    @Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Log.d(TAG, "onDraw() called");
        if (mGame == null || mGame.field == null) return;

        if (mBlockSize == 0) {
            if (mGame.field.mWidth < mGame.field.mHeight) {
                mBlockSize = getWidth() / (mGame.field.mWidth + 1);
            } else {
                mBlockSize = getHeight() / (mGame.field.mHeight + 3);
            }
        }

        // background
        mPaint.setColor(Color.WHITE);
        canvas.drawRect(0, 0, getWidth(), getHeight(), mPaint);

        // background lines
        mPaint.setStrokeWidth(mLineWidth);
        mPaint.setColor(Color.parseColor("#CCCCEE"));
        if (mBackgroundLines == null) {
            mBackgroundLines = new float[4 * (mGame.field.mWidth + 1 + mGame.field.mHeight + 3)];
            for (int i = 0; i < mGame.field.mWidth + 1; i++) {
                mBackgroundLines[4 * i] = mBlockSize / 2 + mBlockSize * i;
                mBackgroundLines[4 * i + 1] = 0;
                mBackgroundLines[4 * i + 2] = mBlockSize / 2 + mBlockSize * i;
                mBackgroundLines[4 * i + 3] = getHeight();
            }
            for (int i = 0; i < mGame.field.mHeight + 3; i++) {
                mBackgroundLines[4 * (mGame.field.mWidth + 1 + i)] = 0;
                mBackgroundLines[4 * (mGame.field.mWidth + 1 + i) + 1] = mBlockSize / 2 + mBlockSize * i;
                mBackgroundLines[4 * (mGame.field.mWidth + 1 + i) + 2] = getWidth();
                mBackgroundLines[4 * (mGame.field.mWidth + 1 + i) + 3] = mBlockSize / 2 + mBlockSize * i;
            }
        }
        canvas.drawLines(mBackgroundLines, mPaint);

        // field lines
        mPaint.setColor(Color.BLACK);
        List<Field.Line> lines = mGame.field.getLines();
        if (mCachedFieldLines == null) {
            List<Field.Line> fieldLines = new ArrayList<Field.Line>();
            for (Field.Line line : lines) {
                if (line.player == Cpu.Player.NONE) fieldLines.add(line);
            }
            mCachedFieldLines = new float[fieldLines.size() * 4];
            for (int i = 0; i < fieldLines.size(); i++) {
                Field.Line line = fieldLines.get(i);
                mCachedFieldLines[4 * i] = mBlockSize / 2 + mBlockSize * line.a.x;
                mCachedFieldLines[4 * i + 1] = 3 * mBlockSize / 2 + mBlockSize * line.a.y;
                mCachedFieldLines[4 * i + 2] = mBlockSize / 2 + mBlockSize * line.b.x;
                mCachedFieldLines[4 * i + 3] = 3 * mBlockSize / 2 + mBlockSize * line.b.y;
            }
        }
        canvas.drawLines(mCachedFieldLines, mPaint);

        // player one's lines
        mPaint.setColor(mColor2);
        if (mCachedFieldLinesOne == null || mFieldChanged) {
            List<Field.Line> edgesOne = new ArrayList<Field.Line>();
            for (Field.Line line : lines) {
                if (line.player == Cpu.Player.ONE) edgesOne.add(line);
            }
            mCachedFieldLinesOne = new float[edgesOne.size() * 4];
            for (int i = 0; i < edgesOne.size(); i++) {
                Field.Line line = edgesOne.get(i);
                mCachedFieldLinesOne[4 * i] = mBlockSize / 2 + mBlockSize * line.a.x;
                mCachedFieldLinesOne[4 * i + 1] = 3 * mBlockSize / 2 + mBlockSize * line.a.y;
                mCachedFieldLinesOne[4 * i + 2] = mBlockSize / 2 + mBlockSize * line.b.x;
                mCachedFieldLinesOne[4 * i + 3] = 3 * mBlockSize / 2 + mBlockSize * line.b.y;
            }
        }
        canvas.drawLines(mCachedFieldLinesOne, mPaint);

        // player two's lines
        mPaint.setColor(mColor1);
        if (mCachedFieldLinesTwo == null || mFieldChanged) {
            List<Field.Line> edgesTwo = new ArrayList<Field.Line>();
            for (Field.Line line : lines) {
                if (line.player == Cpu.Player.TWO) edgesTwo.add(line);
            }
            mCachedFieldLinesTwo = new float[edgesTwo.size() * 4];
            for (int i = 0; i < edgesTwo.size(); i++) {
                Field.Line line = edgesTwo.get(i);
                mCachedFieldLinesTwo[4 * i] = mBlockSize / 2 + mBlockSize * line.a.x;
                mCachedFieldLinesTwo[4 * i + 1] = 3 * mBlockSize / 2 + mBlockSize * line.a.y;
                mCachedFieldLinesTwo[4 * i + 2] = mBlockSize / 2 + mBlockSize * line.b.x;
                mCachedFieldLinesTwo[4 * i + 3] = 3 * mBlockSize / 2 + mBlockSize * line.b.y;
            }
        }
        canvas.drawLines(mCachedFieldLinesTwo, mPaint);

        // draft lines
        if (mDraftLines == null || mDraftLinesChanged) {
            List<Field.Edge> edgeList = mGame.getDraftEdges();
            if (!edgeList.isEmpty()) mDraftPlayer = edgeList.get(0).player;
            mDraftLines = new float[edgeList.size()*4];
            for (int i=0;i< edgeList.size();i++) {
                Field.Line line = edgeList.get(i).toLine(mGame.field);
                mDraftLines[4 * i] = mBlockSize / 2 + mBlockSize * line.a.x;
                mDraftLines[4 * i + 1] = 3 * mBlockSize / 2 + mBlockSize * line.a.y;
                mDraftLines[4 * i + 2] = mBlockSize / 2 + mBlockSize * line.b.x;
                mDraftLines[4 * i + 3] = 3 * mBlockSize / 2 + mBlockSize * line.b.y;
            }
        }
        mPaint.setStrokeWidth(mBoldLineWidth);
        if (mDraftLines.length > 0) {
            mPaint.setColor(mDraftPlayer == Cpu.Player.ONE? mColor2 : mColor1);
            canvas.drawLines(mDraftLines, mPaint);
        }

        // ball current position
        mPaint.setColor(mGame.currentPlayer== Cpu.Player.ONE? mColor2 : mColor1);
        Point p = mGame.field.getPosition(mGame.field.ball);
        canvas.drawCircle(mBlockSize / 2 + mBlockSize * p.x, 3 * mBlockSize / 2 + mBlockSize * p.y, mDraftBall ?2* mPointRadius : mPointRadius, mPaint);

        // 'not drawn' edge yet
        if (mNotDrawnLine != null) {
            canvas.drawLine(mNotDrawnLine.a.x, mNotDrawnLine.a.y, mNotDrawnLine.b.x, mNotDrawnLine.b.y, mPaint);
        }

        mFieldChanged = false;
        mDraftLinesChanged = false;
        Log.d(TAG, "onDraw() finished");
    }

    private Point lastPoint;

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if (mListener == null || mGame == null || mGame.field == null) return false;
        if (mDraftBall) {
            if (e.getAction() == MotionEvent.ACTION_UP) {
                Point point = new Point((int)e.getX(),(int)e.getY());
                Point ballPosition = mGame.field.getPosition(mGame.field.ball);
                ballPosition.x = mBlockSize /2 + mBlockSize * ballPosition.x;
                ballPosition.y = 3* mBlockSize /2 + mBlockSize * ballPosition.y;
                if (getDistance(point,ballPosition) <= mBlockSize) {
                    mListener.onBallTapped();
                }
            }
        } else if (!mGame.isOver()) {
            if (e.getAction() == MotionEvent.ACTION_DOWN) {
                lastPoint = new Point((int)e.getX(),(int)e.getY());
            } else if (e.getAction() == MotionEvent.ACTION_MOVE) {
                Point point = new Point((int)e.getX(),(int)e.getY());
                if (getDistance(lastPoint,point) >= mBlockSize /2) {
                    Point nextPoint = getNextPoint(lastPoint,point);
                    mListener.onNextPointCalculated(nextPoint);
                }
            } else if (e.getAction() == MotionEvent.ACTION_UP) {
                Point point = new Point((int)e.getX(),(int)e.getY());
                Point nextPoint = null;
                if (getDistance(lastPoint,point) >= mBlockSize /2) {
                    nextPoint = getNextPoint(lastPoint,point);
                }
                mListener.onUp(nextPoint);
            }
        }
        return true;
    }

    private static double getDistance(Point p1, Point p2) {
        return Math.sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
    }

    private static Point getNextPoint(Point p1, Point p2) {
        double angle = Math.toDegrees(Math.atan2(p2.x - p1.x, p2.y - p1.y));
        if (angle < 0) angle += 360;
        if (angle >= 45-22.5 && angle < 45+22.5) return new Point(1,1);
        else if (angle >= 90-22.5 && angle < 90+22.5) return new Point(1,0);
        else if (angle >= 135-22.5 && angle < 135+22.5) return new Point(1,-1);
        else if (angle >= 180-22.5 && angle < 180+22.5) return new Point(0,-1);
        else if (angle >= 225-22.5 && angle < 225+22.5) return new Point(-1,-1);
        else if (angle >= 270-22.5 && angle < 270+22.5) return new Point(-1,0);
        else if (angle >= 315-22.5 && angle < 315+22.5) return new Point(-1,1);
        else return new Point(0,1);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (mAspectRatioClass == null) {
            super.onMeasure(widthMeasureSpec,heightMeasureSpec);
            return;
        }
        mAspectRatioClass.measure(widthMeasureSpec, heightMeasureSpec);
        setMeasuredDimension(mAspectRatioClass.getMeasuredWidth(), mAspectRatioClass.getMeasuredHeight() );
    }

    public interface FieldViewListener {
        void onBallTapped();
        void onNextPointCalculated(Point pt);
        void onUp(Point pt);
    }

    private static class AspectRatioClass {
        private float aspectRatio;

        public AspectRatioClass(float aspectRatio) {
            this.aspectRatio = aspectRatio;
        }

        public void measure(int widthMeasureSpec, int heightMeasureSpec) {
            measure(widthMeasureSpec, heightMeasureSpec, this.aspectRatio);
        }

        public void measure(int widthMeasureSpec, int heightMeasureSpec, double aspectRatio) {
            int widthMode = MeasureSpec.getMode( widthMeasureSpec );
            int widthSize = widthMode == MeasureSpec.UNSPECIFIED ? Integer.MAX_VALUE : MeasureSpec.getSize( widthMeasureSpec );
            int heightMode = MeasureSpec.getMode( heightMeasureSpec );
            int heightSize = heightMode == MeasureSpec.UNSPECIFIED ? Integer.MAX_VALUE : MeasureSpec.getSize( heightMeasureSpec );

            if ( heightMode == MeasureSpec.EXACTLY && widthMode == MeasureSpec.EXACTLY ) {
                measuredWidth = widthSize;
                measuredHeight = heightSize;
            } else if ( heightMode == MeasureSpec.EXACTLY ) {
                measuredHeight = (int) Math.min( heightSize, widthSize / aspectRatio );
                measuredWidth = (int) (measuredHeight * aspectRatio);
            } else if ( widthMode == MeasureSpec.EXACTLY ) {
                measuredWidth = (int) Math.min( widthSize, heightSize * aspectRatio );
                measuredHeight = (int) (measuredWidth / aspectRatio);
            } else {
                if ( widthSize > heightSize * aspectRatio ) {
                    measuredHeight = heightSize;
                    measuredWidth = (int)( measuredHeight * aspectRatio );
                } else {
                    measuredWidth = widthSize;
                    measuredHeight = (int) (measuredWidth / aspectRatio);
                }
            }
        }

        private Integer measuredWidth = null;

        public int getMeasuredWidth() {
            return measuredWidth;
        }

        private Integer measuredHeight = null;

        public int getMeasuredHeight() {
            return measuredHeight;
        }
    }
}
