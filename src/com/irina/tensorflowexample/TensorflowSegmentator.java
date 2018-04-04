package com.irina.tensorflowexample;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.support.v4.os.TraceCompat;
import android.util.Log;

import org.tensorflow.contrib.android.TensorFlowInferenceInterface;

import java.io.IOException;
import android.graphics.Color;

/**
 * Created by irina on 02.04.18.
 */

public class TensorflowSegmentator {
    private static final String TAG = "ImageSegmentator";

    // Config values.
    private String inputName;
    private String outputName;
    private int inputSize;

    // Pre-allocated buffers.
    private int[] intValues;
    private float[] floatValues;
    private int[] output;

    private TensorFlowInferenceInterface inferenceInterface;

    private boolean runStats = false;
    private int[] labelColours = {0, 250};

    private TensorflowSegmentator() {
    }

    public static TensorflowSegmentator create(
            AssetManager assetManager,
            String modelFilename,
            int inputSize,
            String inputName,
            String outputName)
            throws IOException {
        TensorflowSegmentator s = new TensorflowSegmentator();
        s.inputName = inputName;
        s.outputName = outputName;
        Log.i(TAG, "Before");
        s.inferenceInterface = new TensorFlowInferenceInterface(assetManager, modelFilename);
        Log.i(TAG, "After");

        // The shape of the output is [N, NUM_CLASSES], where N is the batch size.
        //int numClasses =
        //        (int) s.inferenceInterface.graph().operation(outputName).output(0).shape().size(1);
        //Log.i(TAG, "Read, output layer size is " + numClasses);

        // Ideally, inputSize could have been retrieved from the shape of the input operation.  Alas,
        // the placeholder node for input in the graphdef typically used does not specify a shape, so it
        // must be passed in as a parameter.
        s.inputSize = inputSize;

        // Pre-allocate buffers.
        s.intValues = new int[inputSize * inputSize];
        s.floatValues = new float[inputSize * inputSize * 3];
        s.output = new int[inputSize * inputSize * 1];

        return s;
    }

    public int[] recognizeImage(final Bitmap bitmap) {
        // Log this method so that it can be analyzed with systrace.
        TraceCompat.beginSection("segmentImage");

        TraceCompat.beginSection("preprocessBitmap");
        // Preprocess the image data from 0-255 int to normalized float based
        // on the provided parameters.
        bitmap.getPixels(intValues, 0, bitmap.getWidth(), 0, 0, bitmap.getWidth(), bitmap.getHeight());
        for (int i = 0; i < intValues.length; ++i) {
            final int val = intValues[i];
            floatValues[i * 3 + 0] = ((val >> 16) & 0xFF);
            floatValues[i * 3 + 1] = ((val >> 8) & 0xFF);
            floatValues[i * 3 + 2] = (val & 0xFF);
        }
        TraceCompat.endSection();

        // Copy the input data into TensorFlow.
        TraceCompat.beginSection("feed");
        inferenceInterface.feed(
                inputName, floatValues, 1, bitmap.getWidth(), bitmap.getHeight(), 3);
        TraceCompat.endSection();

        // Run the inference call.
        TraceCompat.beginSection("run");
        inferenceInterface.run(new String[] {outputName}, runStats);
        TraceCompat.endSection();

        // Copy the output Tensor back into the output array.
        TraceCompat.beginSection("fetch");
        inferenceInterface.fetch(outputName, output);
        TraceCompat.endSection();
        Log.i(TAG, "Output size: " + output.length);
        // PostProcessing.
        for (int i = 0; i < output.length; ++i) {
            if(output[i] == 1)
                intValues[i] = Color.rgb(255, 0, 0);
            else
                intValues[i] = Color.rgb(0, 0, 0);
        }

        TraceCompat.endSection(); // "segmentImage"
        return intValues;
    }

    public void close() {
        inferenceInterface.close();
    }
}
