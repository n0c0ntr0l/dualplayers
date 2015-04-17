package com.players.jason.dualplayers;

import java.io.Serializable;

/**
 * Created by Jason on 11/04/2015.
 */
public class TrackStats implements Serializable {


    private float trackBPM;
    private double trackBPS;
    private double trackBPMS;
    private int key;
    private String filePath;
    private double trackLength;
    private int sampleRate;
    private int totalNumberOfBeats;
    private int totalNumberOfBars;
    private double firstBeatMs;
    private String camelotKey;
    private int numOfPhasesSixteenBar;

    public static String keyIntToCamelot(int key){
        String camelotKey = "";
        switch (key){
            case 0:
                camelotKey="11B";
                break;
            case 1:
                camelotKey="6B";
                break;
            case 2:
                camelotKey="1B";
                break;
            case 3:
                camelotKey="8B";
                break;
            case 4:
                camelotKey="3B";
                break;
            case 5:
                camelotKey="10B";
                break;
            case 6:
                camelotKey="5B";
                break;
            case 7:
                camelotKey="12B";
                break;
            case 8:
                camelotKey="7B";
                break;
            case 9:
                camelotKey="2B";
                break;
            case 10:
                camelotKey="9B";
                break;
            case 11:
                camelotKey="4B";
                break;
            case 12:
                camelotKey="8A";
                break;
            case 13:
                camelotKey="3A";
                break;
            case 14:
                camelotKey="10A";
                break;
            case 15:
                camelotKey="5A";
                break;
            case 16:
                camelotKey="12A";
                break;
            case 17:
                camelotKey="7A";
                break;
            case 18:
                camelotKey="2A";
                break;
            case 19:
                camelotKey="9A";
                break;
            case 20:
                camelotKey="4A";
                break;
            case 21:
                camelotKey="11A";
                break;
            case 22:
                camelotKey="6A";
                break;
            case 23:
                camelotKey="1A";
                break;

        }

        return camelotKey;
    }

    public TrackStats (String filePath, float trackBPM, int key, double trackLength, double firstBeatMs, int sampleRate){
        this.filePath = filePath;
        this.trackBPM = trackBPM;
        this.key = key;
        this.trackLength = trackLength;
        this.firstBeatMs = firstBeatMs;
        trackBPS = trackBPM / 60;
        trackBPMS = trackBPS / 1000;
        this.sampleRate = sampleRate;
    }

    @Override
    public String toString(){
        return filePath + "," + this.trackBPM + "," + this.key + "," + this.trackLength + "," + totalNumberOfBars;
    }

    public int getSampleRate(){
        return sampleRate;
    }

    public String getFilePath(){
        return this.filePath;
    }

    public float getTrackBPM(){
        return trackBPM;
    }

    public double getFirstBeatMs(){
        return firstBeatMs;
    }

    public int getTotalNumberOfBars(){
        return this.totalNumberOfBars;
    }

    public int[] calcNumberOfBarsPassed(double timePassedInMs){
        int[] barsAndBeats = new int[2];
        int numberOfBeatsPassed = (int)Math.round(((timePassedInMs - firstBeatMs) / 1000) * this.trackBPS);
        int numOfBarsPassed = numberOfBeatsPassed / 4;
        int numberOfBeatsModulus = numberOfBeatsPassed % 4;

        barsAndBeats[0] = numOfBarsPassed;
        barsAndBeats[1] = numberOfBeatsModulus;

        return barsAndBeats;
    }

    public void calcNumberOfSixteenBarPhases(){
        double durationOfTrackInMS = trackLength * 1000;
        double trackLengthRealMS = durationOfTrackInMS - firstBeatMs;
        double numberOfBeats = trackLengthRealMS * trackBPMS;
        double numberOfBars = numberOfBeats / 4;
        totalNumberOfBeats = (int)Math.ceil(numberOfBeats);
        totalNumberOfBars = (int)Math.ceil(numberOfBars);
        this.numOfPhasesSixteenBar = (int)totalNumberOfBars / 16;
    }

    public int[] getNumberOfBarsLeft(double timePassedInMs){
        int[] numOfBarsPast =  calcNumberOfBarsPassed(timePassedInMs);
        int numOfBars = numOfPhasesSixteenBar * 16;
        int[] numOfBarsLeft = new int[2];

        numOfBarsLeft[0] = numOfBars - numOfBarsPast[0];
        switch (numOfBarsPast[1]){
            case 0:
                break;
            case 1:
                numOfBarsLeft[0] = numOfBarsLeft[0] - 1;
                numOfBarsLeft[1] = 4 - numOfBarsPast[1];
                break;
            case 2:
                numOfBarsLeft[0] = numOfBarsLeft[0] - 1;
                numOfBarsLeft[1] = 4 - numOfBarsPast[1];
                break;
            case 3:
                numOfBarsLeft[0] = numOfBarsLeft[0] - 1;
                numOfBarsLeft[1] = 4 - numOfBarsPast[1];
                break;

        }
        return numOfBarsLeft;

    }

    public int getNumOfPhasesSixteenBar(){
        return numOfPhasesSixteenBar;
    }

    public String getCamelotKey(){
        camelotKey = TrackStats.keyIntToCamelot(key);
        return camelotKey;
    }

   /* private void calcNumberOfBeats(){
        double lengthFromFirstToLast = trackLength - (firstBeatMs / 1000);
        double beatsPerSecond = trackBPM / 60;
        totalNumberOfBeats = (int)(Math.ceil(beatsPerSecond * lengthFromFirstToLast));
        totalNumberOfBars = (int) Math.ceil(totalNumberOfBeats / 4);
    } */

    private void setCamelotKey(){
        camelotKey = TrackStats.keyIntToCamelot(key);
    }



}
