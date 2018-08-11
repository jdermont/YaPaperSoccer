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
    public static final int EXPERIMENTAL = 5;

    public enum Player {
        NONE,ONE,TWO
    }

    private int difficulty;
    private FieldOptions fieldOptions;

    public Cpu(int difficulty, FieldOptions fieldOptions) {
        this.difficulty = difficulty;
        this.fieldOptions = fieldOptions;
    }

    public void initialize(int threadsNum) {
        initializeCpu(difficulty,threadsNum);
    }

    public void startGame() {
        int fSize;
        switch (fieldOptions.fieldSize) {
            case Field.SMALL: fSize = 0; break;
            case Field.BIG: fSize = 2; break;
            default: fSize = 1;
        }
        startGame(fSize, fieldOptions.halfLine);
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

    public String getMove(int ruchy) {
        return getBestMove(ruchy);
    }

    public void release() {
        releaseCpu();
    }

    private native void initializeCpu(int difficulty, int threadsNum);

    private native void startGame(int fieldSize, boolean halfLine);

    private native void updateGame(String move);

    private native String getBestMove(int ruchy);

    private native void releaseCpu();

    public static native long[] benchmarkOneGame();

    public static native int benchmarkMCTS(int threadsNum);
}
