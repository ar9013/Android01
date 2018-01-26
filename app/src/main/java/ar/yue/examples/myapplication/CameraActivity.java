package ar.yue.examples.myapplication;

import android.annotation.TargetApi;
import android.app.Activity;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Keep;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;


import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

import java.util.logging.LogRecord;


public class CameraActivity extends Activity implements SurfaceHolder.Callback, Camera.PreviewCallback {


    public static String TAG = "CameraActivity";

    Camera.Size previewFrameSize = null;
    boolean isCameraPreview = false;

    private native String getGPSCoordinates(String rootPath);
    private native void initAR();

    private native void addARRef(int refId, String markerPath);
    private native void detect( int width, int height, int hasAlpha , int isPremultiplied , int lineStride32, byte[] frameData);


    private static final String CAMERA_FRAGMENT = "camera_fragment";

    SurfaceView cameraView, transparentView;
    SurfaceHolder holder, holderTransparent;
    Camera camera;
    int hour = 0;
    int minute = 0;
    int second = 0;

    File features, Keypoints, Descriptors;
    private float RectLeft, RectTop, RectRight, RectBottom;
    int deviceHeight, deviceWidth;
    int[] tempFrameData = null;
    int[] rgbFrameData = null;
    ByteArrayOutputStream out = new ByteArrayOutputStream();

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.activity_main);
        cameraView = (SurfaceView) findViewById(R.id.CameraView);

        holder = cameraView.getHolder();
        holder.addCallback((SurfaceHolder.Callback) this);
        cameraView.setSecure(true);

        transparentView = (SurfaceView) findViewById(R.id.TransparentView);
        holderTransparent = transparentView.getHolder();
        holderTransparent.addCallback((SurfaceHolder.Callback) this);
        holderTransparent.setFormat(PixelFormat.TRANSLUCENT);
        transparentView.setZOrderMediaOverlay(true);

        deviceWidth = getScreenWidth();

        deviceHeight = getScreenHeight();
        tempFrameData = new int[deviceHeight * deviceWidth];
        rgbFrameData = new int[deviceHeight * deviceWidth];

        Log.d(TAG, "deviceHeight :" + deviceHeight);
        Log.d(TAG, "deviceWidth :" + deviceWidth);

        Descriptors = new File("/data/data/" + getPackageName() + "/descriptors");
        Keypoints = new File("/data/data/" + getPackageName() + "/keypoints");
        AssetManager assetManager = this.getAssets();

        initAR();

        copyAssetFolder(assetManager, "keypoints", Keypoints.getAbsolutePath());
        copyAssetFolder(assetManager, "descriptors", Descriptors.getAbsolutePath());

        addARRef(6,  Descriptors.getAbsolutePath()+"/006.stats.yaml.bin");

    }


    private static boolean copyAssetFolder(AssetManager assetManager, String fromAssetPath, String toPath) {

        try {
            String[] files = assetManager.list(fromAssetPath);
            new File(toPath).mkdirs();
            boolean res = true;
            for (String file : files)
                if (file.contains("."))
                    res &= copyAsset(assetManager, fromAssetPath + "/" + file, toPath + "/" + file);
                else
                    res &= copyAssetFolder(assetManager,
                            fromAssetPath + "/" + file,
                            toPath + "/" + file);
            return res;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private static boolean copyAsset(AssetManager assetManager,  String fromAssetPath, String toPath) {
        InputStream in = null;
        OutputStream out = null;
        try {
            in = assetManager.open(fromAssetPath);
            new File(toPath).createNewFile();
            out = new FileOutputStream(toPath);
             Log.d(TAG,"COPY FILE :"+toPath);
            copyFile(in, out);
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
            return true;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while((read = in.read(buffer)) != -1){
            out.write(buffer, 0, read);
        }
    }

    public static int getScreenWidth() {

        return Resources.getSystem().getDisplayMetrics().widthPixels;
    }


    public static int getScreenHeight() {

        return Resources.getSystem().getDisplayMetrics().heightPixels;
    }

    public void refreshCamera() {

        if (holder.getSurface() == null) {

            return;
        }

        try {
            camera.stopPreview();
            isCameraPreview = false;

        } catch (Exception e) {

        }

        try {
            camera.setPreviewDisplay(holder);
            camera.setPreviewCallback(this);
            camera.startPreview();
            isCameraPreview = true;

            // 更新 camera 的時候畫上去
            Draw(20,200,100,250,120,30,15,40);

        } catch (Exception e) {

        }
    }


    private void Draw(float BLx, float BLy, float BRx, float BRy, float TLx, float TLy , float TRx, float TRy) {
        Canvas canvas = holderTransparent.lockCanvas(null);

        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setStyle(Paint.Style.STROKE);
        paint.setColor(Color.GREEN);
        paint.setStrokeWidth(3);

        canvas.drawLine(BLx,BLy,BRx,BRy,paint);

        paint.setColor(Color.YELLOW);
        canvas.drawLine(BRx,BRy,TRx,TRy,paint);

        paint.setColor(Color.BLUE);
        canvas.drawLine(TRx,TRy,TLx,TLy,paint);

        paint.setColor(Color.CYAN);
        canvas.drawLine(TLx,TLy,BLx,BLy,paint);


        paint.setColor(Color.RED);
        paint.setTextSize(40);
        canvas.drawText(getGPSCoordinates(getFilesDir().getAbsolutePath()), 20, 40, paint);

        holderTransparent.unlockCanvasAndPost(canvas);
    }


    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        try {

            camera = Camera.open(); //open a camera

        } catch (Exception e) {

            Log.i("Exception", e.toString());

            return;
        }

        Camera.Parameters param;

        param = camera.getParameters();

        Display display = ((WindowManager) getSystemService(WINDOW_SERVICE)).getDefaultDisplay();

        if (display.getRotation() == Surface.ROTATION_0) {
            camera.setDisplayOrientation(90);
        }
        camera.setParameters(param);

        try {
            camera.setPreviewDisplay(holder);

            camera.startPreview();
            isCameraPreview = true;

        } catch (Exception e) {
            return;
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        refreshCamera(); //call method for refress camera


    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }



    @Override
    protected void onPause() {
        super.onPause();

        //一定要设置为空
    //    camera.setPreviewCallback(null);
      //  camera.stopPreview();
       // camera.release();
       // camera = null;
    }


    @Override
    protected void onDestroy() {

        super.onDestroy();
        // 釋放 相機
        holder.removeCallback(this);
        holderTransparent.removeCallback(this);
        holder.removeCallback(this);
        camera = null;
    }

    public static void UpdateARID(int redId){

        Log.d(TAG,"UpdateARID :"+redId);
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    @Override
    public void onPreviewFrame(byte[] yuvNV21FrameData, Camera camera) {

        // Log.d(TAG,"onPreviewFrame");
        // convert yuvNV21 to Rgb

        Camera.Size size = camera.getParameters().getPreviewSize();
        int width = size.width;
        int height = size.height;

        Log.d(TAG,"width :"+width);
        Log.d(TAG,"height :"+height);

        YuvImage yuvImage = new YuvImage(yuvNV21FrameData, ImageFormat.NV21, deviceWidth, deviceHeight, null);
        yuvImage.compressToJpeg(new Rect(0, 0, deviceWidth, deviceHeight), 50, out);

        Log.d(TAG,"frame size :"+ yuvNV21FrameData.length);

        Bitmap bmp =  BitmapFactory.decodeByteArray(out.toByteArray(),0,out.toByteArray().length);

        boolean hasAlpha  = bmp.hasAlpha();
        boolean isPremultiplied = bmp.isPremultiplied();

        int hasAlphaJni = (hasAlpha) ? 1 : 0;
        int isPremultipliedJni = (isPremultiplied) ? 1 : 0;

        Log.d(TAG,"hasAlpha :"+hasAlphaJni);
        Log.d(TAG,"isPremultiplied :"+isPremultipliedJni);

            // 傳到 iCRT
        detect(width , height, 0 , 0 , 0, yuvNV21FrameData);
        out.reset();
    }

    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            Log.d(TAG,"messageMe:"+msg.obj);
        }
    };


    static {
        System.loadLibrary("test-boost");
    }

}
