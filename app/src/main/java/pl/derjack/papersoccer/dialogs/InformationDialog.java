package pl.derjack.papersoccer.dialogs;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AlertDialog;

import pl.derjack.papersoccer_single.R;

public class InformationDialog extends DialogFragment {
    public static final String TAG = "InformationDialog";

    private String mTitle = "Information Dialog";
    private String mMessage = "Some message";
    private DialogInterface.OnClickListener mListener;

    public InformationDialog() {
        setRetainInstance(true);
    }

    public void setTitle(String title) {
        this.mTitle = title;
    }

    public void setMessage(String message) {
        this.mMessage = message;
    }

    public void setOnClickListener(DialogInterface.OnClickListener listener) {
        this.mListener = listener;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
        alertDialogBuilder.setTitle(mTitle);
        alertDialogBuilder.setMessage(mMessage);
        alertDialogBuilder.setPositiveButton(R.string.ok, mListener);
        return alertDialogBuilder.create();
    }

    @Override
    public void onResume() {
        super.onResume();
        getDialog().setCancelable(false);
        getDialog().setCanceledOnTouchOutside(false);
    }

    @Override
    public void onDestroyView() {
        if (getDialog() != null && getRetainInstance()) {
            getDialog().setDismissMessage(null);
        }
        super.onDestroyView();
    }
}
