package com.players.jason.dualplayers;

/**
 * Created by Jason on 16/04/2015.
 */
public class PlayerJNICom {

    static {
        System.loadLibrary("DualPlayers");
    }



    public native void crossFaderController(int value);

    public native void toggleDeckA();

    public native void toggleDeckB();

    public native void addNewTrackDeckA(String pathToFile, double[] values);

    public native void addNewTrackDeckB(String pathToFile, double[] values);

    public native void lowKillDeckA(int value);

    public native void lowKillDeckB(int value);

    public native double positionOfDeckAInMs();

    public native double positionOfDeckBInMs();

    public native void initializeAll(int theBufferSize);

    public native void setSampleRate(int theSampleRate);
}
