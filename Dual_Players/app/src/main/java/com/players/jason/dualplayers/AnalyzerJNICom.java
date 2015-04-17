package com.players.jason.dualplayers;

/**
 * Created by Jason on 14/04/2015.
 */
public class AnalyzerJNICom {

    static {
        System.loadLibrary("DualPlayers");
    }

    String pathOfFile;


    private float bpm;

    public AnalyzerJNICom(String pathOfFile){
            this.pathOfFile = pathOfFile;
    }

    public void analyse(){
        getPathOfFile(pathOfFile);

    }

    public float returnBPM(){
        bpm = getBPMofFile();
        System.out.println("about to get bpm from java end " + bpm);
        return bpm;
    }

    public double returnTrackLength(){

        double length = getTimeLengthOfFile();
        System.out.println("THIS IS THE LENGTH: " + length);
        return length;
    }

    public boolean returnIsTrackAnalysed(){
        return getIsFileAnalysed();
    }

    public int returnTheSampleRate(){
        int sampleRate = getSampleRate();
        return sampleRate;
    }

    public float returnTheFirstBeatMs(){
        float firstBMS =  returnFirstBeatMs();
        System.out.println("THIS IS THE bms: " + firstBMS);
        return firstBMS;
    }

    public int returnTheFileKey(){
        int key = getFileKey();
        System.out.println(key  + " IS THE KEY");
        return key;
    }

    private native int getSampleRate();
    private native void getPathOfFile(String pathOfFile);
    private native float getBPMofFile();
    private native double getTimeLengthOfFile();
    private native boolean getIsFileAnalysed();
    private native float returnFirstBeatMs();
    private native int getFileKey();
    public native void destroyAnalyzer();
}
