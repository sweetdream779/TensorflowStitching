package com.irina.tensorflowexample;

import android.graphics.Bitmap;

import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;

/**
 * Created by irina on 04.12.17.
 */

public class Stitching {
    private static byte[] bitmapToBytes(Bitmap bmp)
    {
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bmp.compress(Bitmap.CompressFormat.PNG, 100, stream);
        return stream.toByteArray();
    }

    private static byte[] stringToBytes(String s) {
        return s.getBytes(StandardCharsets.US_ASCII);
    }

    public native void stitchImages(byte[] data, byte[] data2,
                                    byte[] data3, byte[] data4,
                                    int num_homo, boolean useGdf, long matAddr);

    public void StitchImages(String impath1, String impath2, String segpath1, String segpath2, boolean use_gdf, int homoNum, long matAddr) {
        stitchImages(stringToBytes(impath1), stringToBytes(impath2),
                    stringToBytes(segpath1), stringToBytes(segpath2),
                    homoNum, use_gdf, matAddr);
    }
}
