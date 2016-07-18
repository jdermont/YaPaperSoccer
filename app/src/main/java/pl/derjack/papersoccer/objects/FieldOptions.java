package pl.derjack.papersoccer.objects;

import android.os.Parcel;
import android.os.Parcelable;

public class FieldOptions implements Parcelable {
    public int fieldSize;
    public boolean halfLine;

    public FieldOptions(int fieldSize, boolean halfLine) {
        this.fieldSize = fieldSize;
        this.halfLine = halfLine;
    }

    protected FieldOptions(Parcel in) {
        this.fieldSize = (int)in.readSerializable();
        this.halfLine = in.readInt()==1;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeSerializable(fieldSize);
        dest.writeInt(halfLine?1:0);
    }

    public static final Creator<FieldOptions> CREATOR = new Creator<FieldOptions>() {
        @Override
        public FieldOptions createFromParcel(Parcel in) {
            return new FieldOptions(in);
        }

        @Override
        public FieldOptions[] newArray(int size) {
            return new FieldOptions[size];
        }
    };
}
