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

import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.wonderkiln.camerakit.CameraKitError;
import com.wonderkiln.camerakit.CameraKitEvent;
import com.wonderkiln.camerakit.CameraKitEventListener;
import com.wonderkiln.camerakit.CameraKitImage;
import com.wonderkiln.camerakit.CameraKitVideo;
import com.wonderkiln.camerakit.CameraView;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {

    private static final int INPUT_SIZE = 224;
    private static final int IMAGE_MEAN = 117;
    private static final float IMAGE_STD = 1;

    private static final String MODEL_FILE2 = "file:///android_asset/icnet_optimized.pb";
    private static final int INPUT_SIZE2 = 800;
    private static final String INPUT_NAME2 = "input";
    private static final String OUTPUT_NAME2 = "indices";

    private TensorflowSegmentator segmentator;
    private Executor executor = Executors.newSingleThreadExecutor();
    private TextView textViewResult;
    private Button btnToggleCamera, btnSegment;
    private Button btnSelect;
    private ImageView imageViewResult, imageViewSegnmented;
    private CameraView cameraView;
    private static final String LOG_TAG = "TensorflowActivity";
    private static final int REQUEST_IMAGE_SELECT = 200;
    private String imgPath = null;
    private Bitmap bmp;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        cameraView = (CameraView) findViewById(R.id.cameraView);

        imageViewResult = (ImageView) findViewById(R.id.imageViewResult);
        imageViewSegnmented = (ImageView) findViewById(R.id.ivSegmented);

        textViewResult = (TextView) findViewById(R.id.textViewResult);
        textViewResult.setMovementMethod(new ScrollingMovementMethod());

        btnToggleCamera = (Button) findViewById(R.id.btnToggleCamera);
        btnSegment = (Button) findViewById(R.id.btnSegment);

        btnSelect = (Button) findViewById(R.id.btnSelect);
        btnSelect.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                Intent i = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                startActivityForResult(i, REQUEST_IMAGE_SELECT);
            }
        });

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
                bitmap = Bitmap.createScaledBitmap(bitmap, INPUT_SIZE2, INPUT_SIZE2, false);
                imageViewResult.setImageBitmap(bitmap);

                Bitmap.Config conf = Bitmap.Config.ARGB_8888; // see other conf types
                Bitmap bitmap2 = Bitmap.createBitmap(INPUT_SIZE2, INPUT_SIZE2, conf);
                int[] res = segmentator.recognizeImage(bitmap);

                bitmap2.setPixels(res, 0, INPUT_SIZE2, 0, 0, INPUT_SIZE2, INPUT_SIZE2);
                imageViewSegnmented.setImageBitmap(bitmap2);

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

        btnSegment.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cameraView.captureImage();
            }
        });

        initTensorFlowAndLoad();
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
            bmp = Bitmap.createScaledBitmap(bmp, INPUT_SIZE2, INPUT_SIZE2, false);
            imageViewResult.setImageBitmap(bmp);
            Log.d(LOG_TAG, imgPath);

            int[] res = segmentator.recognizeImage(bmp);
            Bitmap.Config conf = Bitmap.Config.ARGB_8888; // see other conf types
            Bitmap bitmap2 = Bitmap.createBitmap(INPUT_SIZE2, INPUT_SIZE2, conf);
            bitmap2.setPixels(res, 0, INPUT_SIZE2, 0, 0, INPUT_SIZE2, INPUT_SIZE2);
            imageViewSegnmented.setImageBitmap(bitmap2);

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
