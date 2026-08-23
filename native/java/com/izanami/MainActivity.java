package com.izanami;
import android.app.NativeActivity;
import android.os.Bundle;
import android.widget.EditText;
import android.text.TextWatcher;
import android.text.Editable;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.content.Context;
import android.widget.FrameLayout;
import android.view.KeyEvent;
import android.widget.TextView;

public class MainActivity extends NativeActivity {
    public static android.app.Activity self;
    private EditText mEdit;
    static { System.loadLibrary("main"); }
    public native void nativeOnText(String text);
    public native void nativeOnEnter();

    protected void onCreate(Bundle b) {
        super.onCreate(b);
        self = this;
        mEdit = new EditText(this);
        mEdit.setBackgroundColor(0x00000000);
        addContentView(mEdit, new FrameLayout.LayoutParams(1, 1));
        mEdit.setImeOptions(EditorInfo.IME_ACTION_SEND);
        mEdit.setSingleLine(true);
        mEdit.addTextChangedListener(new TextWatcher() {
            public void beforeTextChanged(CharSequence s, int a, int c, int d) {}
            public void onTextChanged(CharSequence s, int a, int b, int c) { nativeOnText(s.toString()); }
            public void afterTextChanged(Editable s) {}
        });
        mEdit.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_SEND) { nativeOnEnter(); return true; }
                return false;
            }
        });
    }

    public void showKeyboard() {
        runOnUiThread(new Runnable() { public void run() {
            mEdit.requestFocus();
            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.showSoftInput(mEdit, InputMethodManager.SHOW_IMPLICIT);
        }});
    }

    public void hideKeyboard() {
        runOnUiThread(new Runnable() { public void run() {
            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.hideSoftInputFromWindow(mEdit.getWindowToken(), 0);
        }});
    }

    public void clearEdit() {
        runOnUiThread(new Runnable() { public void run() { mEdit.setText(""); } });
    }
}
