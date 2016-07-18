package pl.derjack.papersoccer.menu;

import android.content.Context;
import android.support.v4.app.Fragment;
import android.view.View;

import pl.derjack.papersoccer.MainActivity;
import pl.derjack.papersoccer.objects.FieldOptions;

public abstract class MenuFragment extends Fragment implements View.OnClickListener {
    protected MainActivity mMainActivity;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mMainActivity = (MainActivity)context;
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mMainActivity = null;
    }

    public void onFieldChosen(FieldOptions fieldOptions) {

    }

}
