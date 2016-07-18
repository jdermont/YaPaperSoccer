package pl.derjack.papersoccer.menu;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.objects.Cpu;

public class BenchmarkFragment extends MenuFragment {
    public static final String TAG = "BenchmarkFragment";

    private final Runnable benchmarkAction = new Runnable() {
        @Override
        public void run() {
            double avg;
            synchronized (BenchmarkFragment.this) {
                avg = mMovesSum / (mTimesSum /1000000.0);
            }
            mStatisticsTxt.setText(getResources().getString(R.string.benchmark_statistics, mCount, Math.round(avg)));
        }
    };
    private long mMovesSum, mTimesSum;
    private int mCount;
    private ExecutorService mExecutor;
    private Handler mHandler;
    private boolean mIsBenchmarking;
    private TextView mStatisticsTxt;
    private Button startStopBtn;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        View rootView = inflater.inflate(R.layout.fragment_benchmark, container, false);
        mStatisticsTxt = (TextView)rootView.findViewById(R.id.statisticsTxt);
        startStopBtn = (Button)rootView.findViewById(R.id.startStopBtn);
        startStopBtn.setOnClickListener(this);
        mHandler = new Handler();
        return rootView;
    }

    @Override
    public void onPause() {
        super.onPause();
        stopBenchmark();
        mHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateViews();
    }

    @Override
    public void onClick(View v) {
        if (mIsBenchmarking) {
            stopBenchmark();
        } else {
            startBenchmark();
        }
        updateViews();
    }

    private void startBenchmark() {
        if (mExecutor != null) {
            mExecutor.shutdownNow();
            try {
                mExecutor.awaitTermination(1000L, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        synchronized (this) {
            mCount = 0;
            mMovesSum = 0;
            mTimesSum = 0;
        }
        mExecutor = Executors.newSingleThreadExecutor();
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                while (!Thread.currentThread().isInterrupted()) {
                    long[] results = Cpu.benchmarkOneGame();
                    if (Thread.currentThread().isInterrupted()) {
                        break;
                    }
                    synchronized (BenchmarkFragment.this) {
                        mMovesSum += results[0];
                        mTimesSum += results[1];
                        mCount++;
                    }
                    mHandler.post(benchmarkAction);
                }
            }
        });
        mIsBenchmarking = true;
    }

    private void stopBenchmark() {
        if (mExecutor != null) {
            mExecutor.shutdownNow();
        }
        mIsBenchmarking = false;
    }

    private void updateViews() {
        double avg;
        synchronized (BenchmarkFragment.this) {
            avg = mMovesSum / (mTimesSum /1000000.0);
        }
        mStatisticsTxt.setText(getResources().getString(R.string.benchmark_statistics, mCount, Math.round(avg)));
        startStopBtn.setText(mIsBenchmarking ? getString(R.string.stop_benchmark) : getString(R.string.start_benchmark));
    }
}
