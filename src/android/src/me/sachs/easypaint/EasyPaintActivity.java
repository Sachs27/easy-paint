package me.sachs.easypaint;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class EasyPaintActivity extends Activity {
	private static String TAG = "EasyPaintActivity";

	EasyPaintView view;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Log.d(TAG, "onCreate");
        this.view = new EasyPaintView(getApplication());
        setContentView(view);
    }

    @Override protected void onPause() {
        super.onPause();
        view.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        view.onResume();
    }
}
