package com.izanami;
import android.app.NativeActivity;
import android.os.Bundle;
import android.widget.EditText;
import android.text.TextWatcher;
import android.text.Editable;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.content.Context;
import android.content.Intent;
import android.widget.FrameLayout;
import android.view.KeyEvent;
import android.widget.TextView;

public class MainActivity extends NativeActivity {
    public static android.app.Activity self;
    private EditText mEdit;
    static { System.loadLibrary("main"); }
    public native void nativeOnText(String text);
    public native void nativeOnEnter();
    public native void nativeOnKeyboard(boolean visible);

    protected void onCreate(Bundle b) {
        super.onCreate(b);
        self = this;
        final android.view.View root = getWindow().getDecorView().getRootView();
        root.getViewTreeObserver().addOnGlobalLayoutListener(new android.view.ViewTreeObserver.OnGlobalLayoutListener() {
            public void onGlobalLayout() {
                android.graphics.Rect r = new android.graphics.Rect();
                root.getWindowVisibleDisplayFrame(r);
                int diff = root.getRootView().getHeight() - r.height();
                nativeOnKeyboard(diff > 200);
            }
        });
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

    private static final int PICK_IMAGE = 4242;
    public void pickImage() {
        runOnUiThread(new Runnable() { public void run() {
            Intent i = new Intent(Intent.ACTION_GET_CONTENT);
            i.setType("image/*");
            startActivityForResult(Intent.createChooser(i, "Select photo"), PICK_IMAGE);
        }});
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == PICK_IMAGE && resultCode == RESULT_OK && data != null && data.getData() != null) {
            final android.net.Uri uri = data.getData();
            new Thread(new Runnable() { public void run() {
                try {
                    new java.io.File("/sdcard/Izanami/cache").mkdirs();
                    java.io.File out = new java.io.File("/sdcard/Izanami/cache/picked.png");
                    java.io.InputStream in = getContentResolver().openInputStream(uri);
                    android.graphics.Bitmap bmp = android.graphics.BitmapFactory.decodeStream(in);
                    in.close();
                    int w = bmp.getWidth(), h = bmp.getHeight();
                    int maxDim = 768;
                    if (w > maxDim || h > maxDim) {
                        float scale = Math.min((float)maxDim/w, (float)maxDim/h);
                        android.graphics.Bitmap scaled = android.graphics.Bitmap.createScaledBitmap(bmp, (int)(w*scale), (int)(h*scale), true);
                        bmp.recycle();
                        bmp = scaled;
                    }
                    java.io.FileOutputStream fos = new java.io.FileOutputStream(out);
                    bmp.compress(android.graphics.Bitmap.CompressFormat.PNG, 90, fos);
                    fos.close();
                    java.io.FileWriter fw = new java.io.FileWriter("/sdcard/Izanami/memory/picked_image.txt");
                    fw.write(out.getAbsolutePath()); fw.close();
                    hideKeyboard();
                } catch (Exception e) {}
            }}).start();
        }
    }
}
