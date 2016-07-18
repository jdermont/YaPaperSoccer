package pl.derjack.papersoccer.dialogs;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AlertDialog;
import android.view.View;
import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import pl.derjack.papersoccer_single.R;
import pl.derjack.papersoccer.menu.MenuFragment;
import pl.derjack.papersoccer.objects.Field;
import pl.derjack.papersoccer.objects.FieldOptions;
import pl.derjack.papersoccer.settings.Settings;

public class FieldChooserDialog extends DialogFragment {
    public static final String TAG = "FieldChooserDialog";

    private MenuFragment mMenuFragment;

    private RadioGroup mRadioGroup;
    private CheckBox mHalfLineCb;

    public void setFieldChooserListener(MenuFragment menuFragment) {
        this.mMenuFragment = menuFragment;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        View layoutView = View.inflate(getActivity(), R.layout.dialog_field_chooser, null);
        mRadioGroup = (RadioGroup)layoutView.findViewById(R.id.radioGroup);
        mHalfLineCb = (CheckBox)layoutView.findViewById(R.id.halfLineCheckBox);
        if (savedInstanceState == null) {
            Settings settings = Settings.getInstance(getActivity());
            int x = settings.getLastChosenFieldSizePosition();
            ((RadioButton) mRadioGroup.getChildAt(settings.getLastChosenFieldSizePosition())).setChecked(true);
            mHalfLineCb.setChecked(settings.getLastChosenHalfLine());
        }
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
        alertDialogBuilder.setTitle(getString(R.string.choose_field));
        alertDialogBuilder.setView(layoutView);
        alertDialogBuilder.setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Settings settings = Settings.getInstance(getActivity());
                int fieldSize;
                switch (mRadioGroup.getCheckedRadioButtonId()) {
                    case R.id.radioSmall:
                        fieldSize = Field.SMALL;
                        settings.setLastChosenFieldSizePosition(0);
                        break;
                    case R.id.radioBig:
                        fieldSize = Field.BIG;
                        settings.setLastChosenFieldSizePosition(2);
                        break;
                    default:
                        fieldSize = Field.NORMAL;
                        settings.setLastChosenFieldSizePosition(1);
                        break;
                }
                boolean halfLine = mHalfLineCb.isChecked();
                settings.setLastChosenHalfLine(halfLine);
                mMenuFragment.onFieldChosen(new FieldOptions(fieldSize,halfLine));
            }
        });
        alertDialogBuilder.setNegativeButton(getString(R.string.cancel),null);
        return alertDialogBuilder.create();
    }

}
