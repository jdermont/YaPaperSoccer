package pl.derjack.papersoccer.objects;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Game {
    public Cpu.Player currentPlayer;
    public Cpu.Player startingPlayer;
    public Field field;
    private FieldOptions mFieldOptions;

    private List<Field.Edge> mDraftEdges;

    public Game(FieldOptions fieldOptions) {
        this.mFieldOptions = fieldOptions;
        startingPlayer = Cpu.Player.TWO; // player/host is player two; but it takes mColor1 :F
        mDraftEdges = new ArrayList<>();
    }

    public void startGame() {
        currentPlayer = startingPlayer==Cpu.Player.ONE?Cpu.Player.ONE:Cpu.Player.TWO;
        startingPlayer = startingPlayer==Cpu.Player.ONE?Cpu.Player.TWO:Cpu.Player.ONE;
        field = new Field(mFieldOptions.fieldSize, mFieldOptions.halfLine);
        mDraftEdges.clear();
    }

    private int nextNode(char c) {
        switch(c) {
            case '5': return field.getNeibghour(-1, 1);
            case '4': return field.getNeibghour(0,1);
            case '3': return field.getNeibghour(1,1);
            case '6': return field.getNeibghour(-1,0);
            case '2': return field.getNeibghour(1,0);
            case '7': return field.getNeibghour(-1,-1);
            case '0': return field.getNeibghour(0,-1);
            case '1': return field.getNeibghour(1,-1);
        }
        return -1;
    }

    public void makeMove(char move) {
        int next = nextNode(move);
        mDraftEdges.add(new Field.Edge(field.ball,next,currentPlayer));
        field.addEdge(field.ball, next, currentPlayer);
        field.ball = next;
    }

    public void makeMove(int next) {
        char move = field.getDistanceChar(field.ball, next);
        mDraftEdges.add(new Field.Edge(field.ball,next,currentPlayer));
        field.addEdge(field.ball,next,currentPlayer);
        field.ball = next;
    }

    public List<Field.Edge> getDraftEdges() {
        List<Field.Edge> playerDraftEdges = new ArrayList<>();
        if (!mDraftEdges.isEmpty()) {
            int i = mDraftEdges.size()-1;
            Cpu.Player player = mDraftEdges.get(i).player;
            while (i >= 0) {
                Field.Edge edge = mDraftEdges.get(i);
                if (edge.player != player) break;
                playerDraftEdges.add(edge);
                --i;
            }
            Collections.reverse(playerDraftEdges);
        }
        return playerDraftEdges;
    }

    public boolean canUndo() {
        if (isOver()) return false;
        if (!mDraftEdges.isEmpty()) {
            return mDraftEdges.get(mDraftEdges.size()-1).player == currentPlayer;
        }
        return false;
    }

    public void undo() {
        Field.Edge edge = mDraftEdges.remove(mDraftEdges.size()-1);
        field.removeEdge(edge.a, edge.b);
        field.ball = edge.a;
    }

    public boolean isOver() {
        if (field == null) return false;
        return field.goal()!=Cpu.Player.NONE||field.isBlocked();
    }

    public Cpu.Player getWinner() {
        if (!isOver()) return Cpu.Player.NONE;
        if (field.goal()!=Cpu.Player.NONE) {
            return field.goal()==Cpu.Player.ONE?Cpu.Player.TWO:Cpu.Player.ONE;
        } else {
            return currentPlayer==Cpu.Player.ONE?Cpu.Player.TWO:Cpu.Player.ONE;
        }
    }

    public boolean toChangeTurn() {
        return !field.isBlocked()&&!field.passNextDone()&&field.goal()==Cpu.Player.NONE;
    }

    public void changePlayer() {
        currentPlayer = currentPlayer==Cpu.Player.ONE?Cpu.Player.TWO:Cpu.Player.ONE;
    }
}
