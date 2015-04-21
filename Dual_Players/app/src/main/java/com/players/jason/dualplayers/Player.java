package com.players.jason.dualplayers;

import android.app.IntentService;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Random;

/**
 * Created by Jason on 12/04/2015.
 */
public class Player extends IntentService {

    Float currentBPM;
    Integer currentKey;
    ArrayList<TrackStats> listOfTracks;



    public Player() {

        super("Player");


    }


    public void onStartCommand() {

    }


    private void getPreviousAnalysedFiles(Intent intent) {

    }

    @Override
    protected void onHandleIntent(Intent intent) {

        String samplerateString = null, buffersizeString = null;
        if (Build.VERSION.SDK_INT >= 17) {
            AudioManager audioManager = (AudioManager) this.getSystemService(Context.AUDIO_SERVICE);
            Log.d("noco", "audio manager started");
            samplerateString = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            buffersizeString = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        }
        if (samplerateString == null) samplerateString = "44100";
        if (buffersizeString == null) buffersizeString = "512";

        listOfTracks =  (ArrayList<TrackStats>)intent.getExtras().get("filesToPlay");
        Selector mySelector = new Selector(listOfTracks,samplerateString,buffersizeString);
    }

    class Selector extends Thread {
        ArrayList<TrackStats> allTracks;
        String filePathOfNextTrack;
        int masterDeck = -1;
        String currentKeyPlaying;
        TrackStats deckATrack;
        TrackStats deckBTrack;
        boolean isDeckAPlaying = false;
        boolean isDeckBPlaying = false;
        PlayerJNICom playerjnicom = new PlayerJNICom();
        ArrayList<TrackStats> filteredList = new ArrayList<TrackStats>();
        TrackStats nextTrack;
        boolean weAreChangingOver = false;
        boolean changeOverLastStage = false;
        int crossFaderValue = 50;
        int bufferSize;
        int sampleRate;


        public Selector(ArrayList<TrackStats> allTracks,String samplerateString, String bufferSize) {
            this.allTracks = allTracks;
            this.sampleRate = Integer.parseInt(samplerateString);
            this.bufferSize = Integer.parseInt(bufferSize);

            startTracks();
        }

        private void startTracks() {
            Random rng = new Random();
            int numOfTrackToSelect = rng.nextInt(allTracks.size());
            String pathToFile = allTracks.get(numOfTrackToSelect).getFilePath();
            this.currentKeyPlaying = allTracks.get(numOfTrackToSelect).getCamelotKey();

           // playerjnicom.setFirstTrack(TrackStats);
         //   playerjnicom.setSecondTrack(TrackStats);
            playerjnicom.setInitialSampleRate(sampleRate);
            playerjnicom.dualplayer(bufferSize);

            Log.d("nOCO","Initialize all method called");
            try {
                Thread.sleep(800);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            deckATrack = allTracks.get(numOfTrackToSelect);
            playerjnicom.setSampleRateDeckA(deckATrack.getSampleRate());
            playerjnicom.addNewTrackDeckA(pathToFile,new double[]{(double)deckATrack.getTrackBPM(),(double)deckATrack.getFirstBeatMs()});

            playerjnicom.crossFaderController(0);
            allTracks.remove(numOfTrackToSelect);
            Log.d("noco", "before we toggle deck A");
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            playerjnicom.toggleDeckA();
            Log.d("noco","after we toggle deck A");
            this.masterDeck = 0;
            isDeckAPlaying = true;

            waitingState();
        }

        private void setNextTrack() {
            String filePath = nextTrack.getFilePath();
            if (masterDeck == 0) {
                deckBTrack = nextTrack;
                Log.d("noco","WE tried to ADDED THE TRACK TO B");
                playerjnicom.setSampleRateDeckB(deckBTrack.getSampleRate());
                playerjnicom.addNewTrackDeckB(filePath,new double[]{(double) nextTrack.getTrackBPM(), nextTrack.getFirstBeatMs()});
                Log.d("noco","WE ADDED THE TRACK TO B");

                allTracks.remove(deckBTrack);
            } else {
                Log.d("noco","WE Tried THE TRACK TO A");
                deckATrack = nextTrack;
                playerjnicom.setSampleRateDeckA(nextTrack.getSampleRate());
                playerjnicom.addNewTrackDeckA(filePath, new double[]{(double) deckATrack.getTrackBPM(), deckATrack.getFirstBeatMs()});
                allTracks.remove(deckATrack);
                Log.d("noco","WE ADDED THE TRACK TO A");
            }

        }

        private void playNextTrack() {
            if (masterDeck == 0) {
                Log.d("noco","Before B is played");
                playerjnicom.toggleDeckB();
                Log.d("noco","After B is Played");
                //killLowDeckA();
                weAreChangingOver = true;
            } else {
                Log.d("noco","Before A is played");
                playerjnicom.toggleDeckA();
                Log.d("noco","After we Play A");
                //killLowDeckB();
                weAreChangingOver = true;
            }
        }


        private void waitingState() {
            int numOfBarsLeft = getCurrentBarOfMasterTrack()[0];
            int numberOfBeats = getCurrentBarOfMasterTrack()[1];
            Log.d("noco", "bars: " + getCurrentBarOfMasterTrack()[0] + " beats " + getCurrentBarOfMasterTrack()[1]);
            if (((numOfBarsLeft) % 16 == 15) && (numberOfBeats == 3)) {

                shouldISwitch(numOfBarsLeft);
                /*if (changeOverLastStage) {
                    Log.d("noco","Selecting next track" );

                }*/
            }
            if (weAreChangingOver) {
                if (numberOfBeats == 3) {
                    crossFadeTracks();
                }
            }

            try {
                Thread.sleep(200);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            waitingState();

        }

        private void shouldISwitch(int numOfBarsLeft) {
            Random rng = new Random();
            double switchThreshold = 0;

            switch (numOfBarsLeft) {
                case 31:
                    switchThreshold = 1;
                    break;
                case 47:
                    switchThreshold = 0.75;
                    break;
                case 63:
                    switchThreshold = 0.5;
                    break;
                case 79:
                    switchThreshold = 0.125;
                    break;
                case 95:
                    switchThreshold = 0.50;
                    break;
                case 111:
                    switchThreshold = 0.50;
                    break;
                case 127:
                    switchThreshold = 0.50;
                    break;
                case 143:
                    switchThreshold = 0.50;
                    break;
            }
            if (rng.nextDouble() < switchThreshold) {
                Log.d("noco","TRIED TO SWITCH");
                selectNextTrack();
                setNextTrack();
                playNextTrack();

            }
        }


        private void toggleMaster() {
            if (masterDeck == 1) {
                masterDeck = 0;
            } else {
                masterDeck = 1;
            }

        }

        private TrackStats getMasterTrack() {
            if (masterDeck == 0) {
                return deckATrack;
            }
            return deckBTrack;
        }


        private void selectNextTrack() {

            ArrayList<TrackStats> shortList = new ArrayList();
            Log.d("noco", "WE ARE INSIDE SELECT NEXT TRACK");
            for (int i = 0; i < allTracks.size(); i++) {
                if ((allTracks.get(i).getTrackBPM() < (getMasterTrack().getTrackBPM() * 1.08)) && (allTracks.get(i).getTrackBPM() > (getMasterTrack().getTrackBPM() * 0.92))) {
                    Log.d("noco", "WE have added" + allTracks.get(i).getFilePath());
                    shortList.add(allTracks.get(i));
                }
            }

            if(shortList.size() == 0){
                Log.d("nono","NO TRACK FOUND");
            }

            filterListForKeys(shortList, getMasterTrack().getCamelotKey());
            Random rng = new Random();

            nextTrack = filteredList.get(rng.nextInt(filteredList.size()));
        }

        public void filterListForKeys(ArrayList<TrackStats> unFilteredList, String currentTrackKey) {


            CamelotClass[] allKeys = CamelotClass.generateAllKeys();
            Log.d("noco", "THE CURRENT KEY IS: " + currentKeyPlaying);
            CamelotClass currentKey = CamelotClass.getKeyFromString(allKeys, this.currentKeyPlaying);
            CamelotClass[] adjacentKeys = CamelotClass.getNeighbouringKeys(currentKey);

            for (int i = 0; i < unFilteredList.size(); i++) {
                if (unFilteredList.get(i).getCamelotKey().equals(currentTrackKey)) {
                    filteredList.add(unFilteredList.get(i));
                } else {
                    for (int j = 0; j < adjacentKeys.length; j++) {
                        if (unFilteredList.get(i).getCamelotKey().equals(adjacentKeys[i].cellName)) {
                            filteredList.add(unFilteredList.get(i));
                        }
                    }
                }
            }
            if (filteredList.isEmpty()) {
                for (int j = 0; j < adjacentKeys.length; j++) {
                    filterListForKeys(unFilteredList, adjacentKeys[j].cellName);
                }
            }
        }

        private int[] getCurrentBarOfMasterTrack() {

            if (masterDeck == 0) {
                return this.deckATrack.calcNumberOfBarsPassed(playerjnicom.positionOfDeckAInMs());
            } else {
                return deckBTrack.calcNumberOfBarsPassed(playerjnicom.positionOfDeckBInMs());
            }
        }

/*
        private void killLowDeckA() {
            for (int i = 0; i < 101; i++) {
                playerjnicom.lowKillDeckA(i);
                try {
                    Thread.sleep(15);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

        }

        private void killLowDeckB() {
            for (int i = 0; i < 101; i++) {
                playerjnicom.lowKillDeckB(i);
                try {
                    Thread.sleep(15);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        private void setBassOn() {
            if (masterDeck == 0) {
                playerjnicom.lowKillDeckA(0);
            } else {
                playerjnicom.lowKillDeckB(0);
            }
        }
*/
        private void crossFadeTracks() {
            if (masterDeck == 0) {
                crossFaderValue = crossFaderValue + 2;
                playerjnicom.crossFaderController(crossFaderValue);
                if (crossFaderValue > 97) {
                    toggleMaster();
                    playerjnicom.toggleDeckA();
                    isDeckAPlaying = false;
                    weAreChangingOver = false;
                    changeOverLastStage = true;

                }
            } else {
                crossFaderValue = crossFaderValue - 3;
                playerjnicom.crossFaderController(crossFaderValue);

                if (crossFaderValue < 3) {
                    toggleMaster();
                    playerjnicom.toggleDeckB();
                    isDeckBPlaying = false;
                    weAreChangingOver = false;
                    changeOverLastStage = true;
                }
            }
        }



    }

}
