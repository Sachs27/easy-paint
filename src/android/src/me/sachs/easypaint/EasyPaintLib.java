package me.sachs.easypaint;

public class EasyPaintLib {
	static {
		System.loadLibrary("easy-paint-android");
	}
	
	public static native int app_init_window();
	
	public static native int app_init_render_context();
	public static native void app_load_resource(String apk_path);
	/**
	 * @param dt delta time before last update in seconds.
	 */
	public static native void app_on_update(double dt);
	public static native void app_on_render();
	public static native void app_on_resize(int width, int height);
	
	public static native void app_input_manager_touch_down(int n, int[] x, int[] y);
	public static native void app_input_manager_touch_up();
}
