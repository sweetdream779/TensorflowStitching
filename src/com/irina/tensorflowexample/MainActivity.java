/*
 *    Copyright (C) 2017 MINDORKS NEXTGEN PRIVATE LIMITED
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package com.irina.tensorflowexample;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.LinearLayout;

import com.wonderkiln.camerakit.CameraKitError;
import com.wonderkiln.camerakit.CameraKitEvent;
import com.wonderkiln.camerakit.CameraKitEventListener;
import com.wonderkiln.camerakit.CameraKitImage;
import com.wonderkiln.camerakit.CameraKitVideo;
import com.wonderkiln.camerakit.CameraView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import com.irina.tensorflowexample.Stitching;

import org.opencv.core.Mat;

import static org.opencv.android.Utils.matToBitmap;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener{

    private static final int INPUT_SIZE = 224;
    private static final int IMAGE_MEAN = 117;
    private static final float IMAGE_STD = 1;

    private static final String MODEL_FILE2 = "file:///android_asset/icnet_optimized.pb";
    private static final int INPUT_SIZE2 = 800;
    private static final String INPUT_NAME2 = "input";
    private static final String OUTPUT_NAME2 = "indices";

    private TensorflowSegmentator segmentator;
    private Stitching stitcher;

    private Executor executor = Executors.newSingleThreadExecutor();
    private TextView textViewResult;
    private Button btnToggleCamera, btnSegment;
    private Button btnSelect, btnTakePhoto, btnReset, btnStitch;
    private ImageView imOne, imTwo, imResult;
    private LinearLayout imagesLayout;
    private CameraView cameraView;
    private static final String LOG_TAG = "TensorflowActivity";
    private static final int REQUEST_IMAGE_SELECT = 200;
    private String imgPath = null;
    private Bitmap bmp, bitmapOne, bitmapTwo;
    private Bitmap notResized1, notResized2;
    private Bitmap seg1, seg2;

    private TextView seekBarText;
    private SeekBar seekBar;
    private int homoNum;
    private CheckBox checkBox;
    private boolean useGdf;
    private String path1 = null;
    private String path2 = null;
    private String path3 = null;
    private String path4 = null;

    private Mat resultMat;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("mystitcher");
        System.loadLibrary("opencv_java3");
    }

    private String saveToInternalStorage(Bitmap bitmapImage){
        ContextWrapper cw = new ContextWrapper(getApplicationContext());
        // path to /data/data/yourapp/app_data/imageDir
        File directory = cw.getDir("Dir", Context.MODE_PRIVATE);
        // Create imageDir
        File mypath=new File(directory,"profile.jpg");

        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(mypath);
            // Use the compress method on the BitMap object to write image to the OutputStream
            bitmapImage.compress(Bitmap.CompressFormat.PNG, 100, fos);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                fos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return directory.getAbsolutePath();
    }

    public String saveImageToExternalStorage(Bitmap image, String filename) {
        String fullPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "tensorflowStitching";;


            File dir = new File(fullPath);
            if (!dir.exists()) {
                dir.mkdirs();
            }

            final String fname = filename;
            final File file = new File(dir, fname);
            if (file.exists()) {
                file.delete();
            }
            try {
                final FileOutputStream out = new FileOutputStream(file);
                image.compress(Bitmap.CompressFormat.PNG, 100, out);
                out.flush();
                out.close();
                return file.getAbsolutePath();
            } catch (final Exception e) {
                Log.e("saveToExternalStorage()", e.getMessage());
                return "";
            }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        stitcher = new Stitching();
        resultMat = new Mat();
        cameraView = (CameraView) findViewById(R.id.cameraView);

        imOne = (ImageView) findViewById(R.id.imOne);

        imTwo = (ImageView) findViewById(R.id.imTwo);

        imResult = (ImageView) findViewById(R.id.imResult);

        textViewResult = (TextView) findViewById(R.id.textViewResult);
        textViewResult.setMovementMethod(new ScrollingMovementMethod());
        textViewResult.setText("Please take a photo or select one");
        imagesLayout = (LinearLayout) findViewById(R.id.imagesLayout);
        btnToggleCamera = (Button) findViewById(R.id.btnToggleCamera);
        btnSegment = (Button) findViewById(R.id.btnSegment);
        btnSegment.setEnabled(false);
        btnSegment.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                path1 = saveImageToExternalStorage(notResized1,"image1.png");
                path2 = saveImageToExternalStorage(notResized2,"image2.png");

                textViewResult.setText("Segmentation is running, please wait.");
                Bitmap.Config conf = Bitmap.Config.ARGB_8888; // see other conf types

                Bitmap bitmap1 = Bitmap.createBitmap(INPUT_SIZE2, INPUT_SIZE2, conf);
                try {
                    int[] res1 = segmentator.getSegmentation(bitmapOne);
                    bitmap1.setPixels(res1, 0, INPUT_SIZE2, 0, 0, INPUT_SIZE2, INPUT_SIZE2);
                    seg1 = bitmap1;
                    //imOne.setImageBitmap(bitmap1);
                    textViewResult.setText("Segmentation of first image is finished. Wait for second.");
                }catch(Exception e){
                    Log.d(LOG_TAG, e.getMessage());
                    textViewResult.setText("Segmentation error");
                }

                Bitmap bitmap2 = Bitmap.createBitmap(INPUT_SIZE2, INPUT_SIZE2, conf);
                try {
                    int[] res2 = segmentator.getSegmentation(bitmapTwo);
                    bitmap2.setPixels(res2, 0, INPUT_SIZE2, 0, 0, INPUT_SIZE2, INPUT_SIZE2);
                    seg2 = bitmap2;
                    //imTwo.setImageBitmap(bitmap2);
                    textViewResult.setText("Segmentation is finished. Now choose parameters and press 'Stitch images' for images stritching.");
                }catch(Exception e){
                    Log.d(LOG_TAG, e.getMessage());
                    textViewResult.setText("Segmentation error");
                }
                path3 = saveImageToExternalStorage(seg1,"segmentation1.png");
                path4 = saveImageToExternalStorage(seg2,"segmentation2.png");

                btnStitch.setEnabled(true);
                btnSegment.setEnabled(false);
            }
        });

        btnStitch = (Button) findViewById(R.id.btnStitch);
        //btnStitch.setEnabled(false);
        btnStitch.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                textViewResult.setText("Stitching is running..");
                btnReset.setEnabled(false);
                if(checkBox.isChecked())
                    useGdf = true;

                Log.d(LOG_TAG, "PATH: " +path1);
                try {
                    Log.d(LOG_TAG, "PATH: " +path2);
                    Log.d(LOG_TAG, "PATH: " +path3);
                    Log.d(LOG_TAG, "PATH: " +path4);
                    long startTime = System.nanoTime();
                    stitcher.StitchImages(path1, path2, path3, path4, useGdf, homoNum, resultMat.getNativeObjAddr());
                    long elapsedTime = System.nanoTime() - startTime;
                    double seconds = (double)elapsedTime / 1000000000.0;
                    Log.i("STITCHING", "Time taken: " + seconds + " seconds");

                    Bitmap bmpResult = Bitmap.createBitmap(resultMat.cols(), resultMat.rows(),Bitmap.Config.ARGB_8888);
                    matToBitmap(resultMat, bmpResult);
                    imagesLayout.setVisibility(View.GONE);
                    imResult.setVisibility(View.VISIBLE);
                    imResult.setImageBitmap(bmpResult);
                    String pathres = saveImageToExternalStorage(bmpResult,"result.png");
                    textViewResult.setText("Stitching is finished. Press 'Reset images to choose another couple of images.'");
                    //btnStitch.setEnabled(false);
                }catch(Exception e){
                    Log.d(LOG_TAG, e.getMessage());
                    textViewResult.setText("Stitching error");
                }

                btnSegment.setEnabled(false);
                btnReset.setEnabled(true);
            }
        });

        btnReset = (Button) findViewById(R.id.btnReset);
        btnReset.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                textViewResult.setText("Images were removed. Choose another.");
                btnSegment.setEnabled(false);
                //btnStitch.setEnabled(false);
                imResult.setVisibility(View.GONE);
                cameraView.setVisibility(View.VISIBLE);
                imagesLayout.setVisibility(View.GONE);
                imOne.setImageDrawable(null);
                imTwo.setImageDrawable(null);
                btnSelect.setEnabled(true);
                btnToggleCamera.setEnabled(true);
                imResult.setImageDrawable(null);
                btnTakePhoto.setEnabled(true);
            }
        });

        btnTakePhoto = (Button) findViewById(R.id.btnTakePhoto);

        btnSelect = (Button) findViewById(R.id.btnSelect);
        btnSelect.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                Intent i = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                startActivityForResult(i, REQUEST_IMAGE_SELECT);
            }
        });

        seekBar = (SeekBar)findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(this);
        homoNum = 2;//default value
        seekBarText = (TextView)findViewById(R.id.seekBarText);
        seekBarText.setText("Max number of homographies to use for reconstructing(1-10): " + String.valueOf(homoNum));
        checkBox = (CheckBox)findViewById(R.id.checkBox);
        useGdf = false;

        cameraView.addCameraKitListener(new CameraKitEventListener() {
            @Override
            public void onEvent(CameraKitEvent cameraKitEvent) {

            }

            @Override
            public void onError(CameraKitError cameraKitError) {

            }

            @Override
            public void onImage(CameraKitImage cameraKitImage) {

                Bitmap bitmap = cameraKitImage.getBitmap();
                if(imOne.getDrawable() == null) {
                    notResized1 =  Bitmap.createScaledBitmap(bitmap, (int)Math.round(bitmap.getWidth()*0.5),
                                                                     (int)Math.round(bitmap.getHeight()*0.5), false);
                    bitmapOne = Bitmap.createScaledBitmap(bitmap, INPUT_SIZE2, INPUT_SIZE2, false);
                    imOne.setImageBitmap(bitmapOne);
                    textViewResult.setText("First image is selected. Choose second.");
                }
                else if(imTwo.getDrawable() == null) {
                    notResized2 =  Bitmap.createScaledBitmap(bitmap, (int)Math.round(bitmap.getWidth()*0.5),
                                                                     (int)Math.round(bitmap.getHeight()*0.5), false);
                    bitmapTwo = Bitmap.createScaledBitmap(bitmap, INPUT_SIZE2, INPUT_SIZE2, false);
                    imTwo.setImageBitmap(bitmapTwo);
                    cameraView.setVisibility(View.GONE);
                    imagesLayout.setVisibility(View.VISIBLE);
                    //btnSelect.setEnabled(false);
                    btnToggleCamera.setEnabled(false);
                    btnTakePhoto.setEnabled(false);
                    btnSegment.setEnabled(true);
                    textViewResult.setText("Second image is selected. Now press 'Get segmntation' or 'Reset images' to take new photo");
                }

            }

            @Override
            public void onVideo(CameraKitVideo cameraKitVideo) {

            }
        });

        btnToggleCamera.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cameraView.toggleFacing();
            }
        });

        btnTakePhoto.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cameraView.captureImage();
            }
        });

        initTensorFlowAndLoad();
    }
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        homoNum = seekBar.getProgress() + 1;
        seekBarText.setText("Max number of homographies to use for reconstructing(1-10): " + String.valueOf(homoNum));
    }

    @Override
    protected void onResume() {
        super.onResume();
        cameraView.start();
    }

    @Override
    protected void onPause() {
        cameraView.stop();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        executor.execute(new Runnable() {
            @Override
            public void run() {
                segmentator.close();
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_IMAGE_SELECT && resultCode == RESULT_OK) {

            Uri selectedImage = data.getData();
            String[] filePathColumn = {MediaStore.Images.Media.DATA};
            Cursor cursor = MainActivity.this.getContentResolver().query(selectedImage, filePathColumn, null, null, null);
            cursor.moveToFirst();
            int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
            imgPath = cursor.getString(columnIndex);
            cursor.close();
            bmp = BitmapFactory.decodeFile(imgPath);
            if(imOne.getDrawable() == null) {
                notResized1 =  Bitmap.createScaledBitmap(bmp, (int)Math.round(bmp.getWidth()*0.5), (int)Math.round(bmp.getHeight()*0.5), false);
                bitmapOne = Bitmap.createScaledBitmap(bmp, INPUT_SIZE2, INPUT_SIZE2, false);
                imOne.setImageBitmap(bitmapOne);
                textViewResult.setText("First image is selected. Choose second.");
                path1 = imgPath;
            }
            else if(imTwo.getDrawable() == null) {
                notResized2 =  Bitmap.createScaledBitmap(bmp, (int)Math.round(bmp.getWidth()*0.5), (int)Math.round(bmp.getHeight()*0.5), false);
                bitmapTwo = Bitmap.createScaledBitmap(bmp, INPUT_SIZE2, INPUT_SIZE2, false);
                imTwo.setImageBitmap(bitmapTwo);
                //btnSelect.setEnabled(false);
                btnToggleCamera.setEnabled(false);
                btnTakePhoto.setEnabled(false);
                cameraView.setVisibility(View.GONE);
                imagesLayout.setVisibility(View.VISIBLE);
                btnSegment.setEnabled(true);
                textViewResult.setText("Second image is selected. Now press 'Get segmntation' or 'Reset images' to take new photo");
                path2 = imgPath;
            }
            else if(path3 == null){
                path3 = imgPath;
                Log.d(LOG_TAG, "!!!!!!!!!!!!!!!!!!!!!!PATH: " + imgPath);
            }
            else if(path4 == null){
                path4 = imgPath;
                Log.d(LOG_TAG, "!!!!!!!!!!!!!!!!!!!!!!PATH: " +imgPath);
            }
            //Log.d(LOG_TAG, imgPath);

        } else {
            btnSelect.setEnabled(true);
        }

        super.onActivityResult(requestCode, resultCode, data);
    }

    private void initTensorFlowAndLoad() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    segmentator = TensorflowSegmentator.create(
                            getAssets(),
                            MODEL_FILE2,
                            INPUT_SIZE2,
                            INPUT_NAME2,
                            OUTPUT_NAME2);
                    makeButtonVisible();
                }catch(Exception e){
                        Log.d(LOG_TAG, e.getMessage());}

            }
        });
    }

    private void makeButtonVisible() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                btnSegment.setVisibility(View.VISIBLE);
            }
        });
    }
}
