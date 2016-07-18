package pl.derjack.papersoccer.objects;

import java.util.List;

public class Cpu {
    static {
        System.loadLibrary("cpujni");
    }

    public static final int VERY_EASY = 0;
    public static final int EASY = 1;
    public static final int NORMAL = 2;
    public static final int ADVANCED = 3;
    public static final int HARD = 4;

    public enum Player {
        NONE,ONE,TWO
    }

    private int mDifficulty;
    private FieldOptions mFieldOptions;

    public Cpu(int difficulty, FieldOptions fieldOptions) {
        this.mDifficulty = difficulty;
        this.mFieldOptions = fieldOptions;
    }

    public void initialize() {
        initializeCpu(mDifficulty);
    }

    public void startGame() {
        int fSize;
        switch (mFieldOptions.fieldSize) {
            case Field.SMALL: fSize = 0; break;
            case Field.BIG: fSize = 2; break;
            default: fSize = 1;
        }
        startGame(fSize, mFieldOptions.halfLine);
    }

    public void updateGameState(Field field, List<Field.Edge> edges) {
        StringBuilder sb = new StringBuilder(edges.size());
        for (Field.Edge edge : edges) {
            sb.append(field.getDistanceChar(edge.a,edge.b));
        }
        String move = sb.toString();
        updateGame(move);
    }

    public void updateGameState(String move) {
        updateGame(move);
    }

    public String getMove() {
        return getBestMove();
    }

    public void release() {
        releaseCpu();
    }

    private native void initializeCpu(int difficulty);

    private native void startGame(int fieldSize, boolean halfLine);

    private native void updateGame(String move);

    private native String getBestMove();

    private native void releaseCpu();

    public static native long[] benchmarkOneGame();
}
