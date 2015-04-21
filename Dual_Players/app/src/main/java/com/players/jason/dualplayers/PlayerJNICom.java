package com.players.jason.dualplayers;

import android.util.Log;

/**
 * Created by Jason on 16/04/2015.
 */
public class PlayerJNICom {

    static {
        System.loadLibrary("DualPlayers");
    }

    private boolean isDeckAPlaying = false;
    private boolean isDeckBPlaying = false;

    public void toggleDeckA(){
       if(isDeckAPlaying){
           Log.d("noco", "calling pauseDeck A");
           this.pauseDeckA();
           isDeckAPlaying = false;
       } else {
           Log.d("noco","calling playDeck A");
            this.playDeckA();
           isDeckAPlaying = true;
       }
    }

    public void toggleDeckB(){
        if(isDeckBPlaying){
            this.pauseDeckB();
            isDeckBPlaying = false;
        } else {
            this.playDeckB();
            isDeckBPlaying = true;
        }
    }

    public void crossFaderController(int value){
        onCrossfader(value);
    }

    private native void onCrossfader(int value);

    //public native void toggleDeckA();

    //public native void toggleDeckB();

    public native void playDeckA();

    public native void playDeckB();

    public native void pauseDeckA();

    public native void pauseDeckB();

    public native void addNewTrackDeckA(String pathToFile, double[] values);

    public native void addNewTrackDeckB(String pathToFile, double[] values);

    //public native void lowKillDeckA(int value);

    //public native void lowKillDeckB(int value);

    public native double positionOfDeckAInMs();

    public native double positionOfDeckBInMs();

    public native void setSampleRateDeckA(int sampleRate);

    public native void setSampleRateDeckB(int sampleRate);

    public native void setInitialSampleRate(int intialSampleRate);

    public native void dualplayer(int param);

    //public native void initializeAll(int theBufferSize);

    //public native void setSampleRate(int theSampleRate);
}
