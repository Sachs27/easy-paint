package me.sachs.easypaint;

import android.app.Activity;
import android.os.Bundle;

public class EasyPaintActivity extends Activity {
	EasyPaintView view;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
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
