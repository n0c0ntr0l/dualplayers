package com.players.jason.dualplayers;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Environment;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.lang.reflect.Array;
import java.util.ArrayList;


public class DualPlayer extends ActionBarActivity {


    ArrayList<TrackStats> anaylsedFilesDetails = new ArrayList();
    ArrayList<String> listOfAudioFiles = new ArrayList();
    ArrayList<String> listOfFilesToBeAnalysed = new ArrayList();




    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dual_player);
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        getPreviousAnalysedFiles();
        getAllTunes();
        checkIfFileIsAlreadyInList();
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        analyseFiles();

    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_dual_player, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }



    public void getAllTunes(){
        String path = Environment.getExternalStorageDirectory().toString()+"/Music/app/";
        System.out.println("DEBUGJ: " + path);
        Log.d("Files", "Path: " + path);

        File f = new File(path);
        File file[] = f.listFiles();

        for(int i = 0; i < file.length; i++){
            if(file[i].getName().contains(".mp3")){
                if(file[i].length() > 1000){
                    listOfAudioFiles.add(path + file[i].getName());
                    System.out.println(path + file[i].getName());
                }
            }
        }
    }

    private void getPreviousAnalysedFiles(){
        File f = new File(Environment.getExternalStorageDirectory().toString()+"/Dual Player/analysedFiles.txt");
        if(f.exists()){


        try {
            ObjectInputStream objectInputStream = new ObjectInputStream(
                    new FileInputStream(Environment.getExternalStorageDirectory().toString()+"/Dual Player/analysedFiles.txt"));
            anaylsedFilesDetails = (ArrayList<TrackStats>)objectInputStream.readObject();
            Log.d("N0c0","THE SIZE OF ANALYZED FILES IS: " + anaylsedFilesDetails.size());
            objectInputStream.close();

        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    }


    public void analyseFiles(){


        AnalyzeFiles anaylsisThread = new AnalyzeFiles(listOfFilesToBeAnalysed,anaylsedFilesDetails,this);
        anaylsisThread.execute();
    }

    public void checkIfFileIsAlreadyInList(){

        System.out.println("XGFT list of audio files size: " + listOfAudioFiles.size());
        for(int i = 0; i < listOfAudioFiles.size(); i++ ) {
            listOfFilesToBeAnalysed.add(listOfAudioFiles.get(i));

        }


        for(int i = 0; i < anaylsedFilesDetails.size(); i++){
            for(int j = 0; j < listOfFilesToBeAnalysed.size(); j++){
                if(anaylsedFilesDetails.get(i).getFilePath().equals(this.listOfFilesToBeAnalysed.get(j))){
                    listOfFilesToBeAnalysed.remove(j);
                    break;
                }
            }
        }
        if(listOfFilesToBeAnalysed.size() == 0){
            Log.d("noco","we did the fast skip");
            Intent intent = new Intent(this.getApplicationContext(), Player.class);
            intent.putExtra("filesToPlay",anaylsedFilesDetails);
            this.getApplicationContext().startService(intent);
        }
    }





    class AnalyzeFiles extends AsyncTask<Void,Void,Void> {

        ArrayList<String> listOfFilesToBeAnalyzed;
        ArrayList<TrackStats> anaylsedFilesDetails;
        ArrayList<TrackStats> finalAnalysedFiles;
        boolean allFilesAnalysed = false;
        Context context;
        Activity activity;
        AnalyzerJNICom analyzer = null;


        AnalyzeFiles(ArrayList<String> listOfFilesToBeAnalysed, ArrayList<TrackStats> anaylsedFilesDetails, Activity activity){
            this.listOfFilesToBeAnalyzed = listOfFilesToBeAnalysed;
            this.anaylsedFilesDetails = anaylsedFilesDetails;
            this.activity = activity;
        }




        @Override
        protected Void doInBackground(Void... params) {

            for(int i = 0; i < listOfFilesToBeAnalyzed.size(); i++){
                System.out.println(listOfFilesToBeAnalyzed.get(i) + " DEBUG X");
                analyzeFile(listOfFilesToBeAnalyzed.get(i));
            }
            System.out.println( " DEBUG Y");
            allFilesAnalysed = true;
            storeAnalysedFileList();
            Intent intent = new Intent(activity.getApplicationContext(),Player.class);
            this.getPreviousAnalysedFiles();
            intent.putExtra("filesToPlay",anaylsedFilesDetails);
            startService(intent);
            return null;
        }





        private void getPreviousAnalysedFiles(){
            File f = new File(Environment.getExternalStorageDirectory().toString()+"/Dual Player/analysedFiles.txt");
            if(f.exists()){


                try {
                    ObjectInputStream objectInputStream = new ObjectInputStream(
                            new FileInputStream(Environment.getExternalStorageDirectory().toString()+"/Dual Player/analysedFiles.txt"));
                    finalAnalysedFiles = (ArrayList<TrackStats>)objectInputStream.readObject();
                    Log.d("N0c0","THE SIZE OF ANALYZED FILES IS: " + finalAnalysedFiles.size());
                    objectInputStream.close();

                } catch (IOException e) {
                    e.printStackTrace();
                } catch (ClassNotFoundException e) {
                    e.printStackTrace();
                }
            }

        }




        @Override
        protected void onPostExecute(Void nothing){

        }

        private void analyzeFile(String pathToFile){


            if(analyzer == null){
                analyzer = new AnalyzerJNICom(pathToFile);
            }

            analyzer.analyse();





            /*while(!analyzer.returnIsTrackAnalysed()){
                try {
                    Thread.sleep(300);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }*/



            anaylsedFilesDetails.add(new TrackStats(pathToFile,analyzer.returnBPM(),analyzer.returnTheFileKey(),analyzer.returnTrackLength(),analyzer.returnTheFirstBeatMs(),analyzer.returnTheSampleRate()));
            analyzer.destroyAnalyzer();
            System.out.println(anaylsedFilesDetails.get(anaylsedFilesDetails.size() - 1).toString() + "TRACK INFO");

        }


        private void storeAnalysedFileList(){

            try{
            ObjectOutputStream objectOutputStream = new ObjectOutputStream(
                    new FileOutputStream(Environment.getExternalStorageDirectory().toString()+"/Dual Player/analysedFiles.txt"));
                    objectOutputStream.writeObject(anaylsedFilesDetails);
                objectOutputStream.flush();
                objectOutputStream.close();
        } catch (FileNotFoundException ex) {
                ex.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
    }




}
