/*package com.players.jason.dualplayers;
import android.app.Activity;
import android.os.Environment;

import javazoom.jl.converter.Converter;
import javazoom.jl.decoder.DecoderException;
import javazoom.jl.decoder.JavaLayerException;

/**
 * Created by Jason on 25/03/2015.

public class FileAnalyser extends Activity {

    String filePath;
    int sampleRate;
    int lengthOfFile;
    String fileType;

    public FileAnalyser(String filePath){
        this.filePath = filePath;
        fileType =  filePath.split(".")[filePath.split(".").length - 1];
    }

    public void getTrackKey(){

    }



    public void getBPMOfFile(){

    }

    public void decodeMP3(){
        Converter converter = null;
        if(fileType.equals("mp3")){
            converter = new Converter();
            try {
                converter.convert(filePath, Environment.getExternalStorageDirectory().toString() + "/Music/Temp/" + filePath.split(".")[0] + "wav");
            } catch (JavaLayerException e) {

            }


        }
    }


}
*/