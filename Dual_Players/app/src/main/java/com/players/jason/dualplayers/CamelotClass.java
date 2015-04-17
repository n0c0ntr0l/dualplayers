package com.players.jason.dualplayers;

/**
 * Created by Jason on 12/04/2015.
 */
public class CamelotClass {

    CamelotClass leftCell;
    CamelotClass rightCell;
    CamelotClass hourCell;

    public boolean isInner;
    public String cellName;

    public CamelotClass(String cellName){
        this.cellName = cellName;
        if(cellName.contains("B")){
            isInner = true;
        } else {
            isInner = false;
        }
    }

    public void setLeftCell(CamelotClass cell){
        leftCell = cell;
    }

    public void setRightCell(CamelotClass cell){
        rightCell = cell;
    }

    public void setHourCell(CamelotClass cell){
        hourCell = cell;
    }

    public static CamelotClass[] generateAllKeys(){
        CamelotClass[] allKeys = new CamelotClass[24];
        int j = 0;
        for(int i = 1; i < 13; i++){
             String cellName = i + "A";
             allKeys[j] = new CamelotClass(cellName);
             j++;
        }

        for(int i = 1; i < 13; i++){
            String cellName = i + "B";
            allKeys[j] = new CamelotClass(cellName);
            j++;
        }

        String tempCellName;

        for(int i = 0; i < 12; i++){
            if(i == 0){
                allKeys[i].setLeftCell(allKeys[11]);
                allKeys[i].setRightCell(allKeys[1]);
                allKeys[i].setHourCell(allKeys[13]);
            } else if(i == 11) {
                allKeys[i].setLeftCell(allKeys[10]);
                allKeys[i].setRightCell(allKeys[0]);
                allKeys[i].setHourCell(allKeys[i+12]);
            } else {
                allKeys[i].setLeftCell(allKeys[i-1]);
                allKeys[i].setRightCell(allKeys[i+1]);
                allKeys[i].setHourCell(allKeys[i+12]);
            }

        }
        for(int i = 12; i < 24; i++){
            if(i == 12){
                allKeys[i].setLeftCell(allKeys[23]);
                allKeys[i].setRightCell(allKeys[i+1]);
                allKeys[i].setHourCell(allKeys[i-12]);
            } else if(i == 23) {
                allKeys[i].setLeftCell(allKeys[i-1]);
                allKeys[i].setRightCell(allKeys[12]);
                allKeys[i].setHourCell(allKeys[i-12]);
            } else {
                allKeys[i].setLeftCell(allKeys[i-1]);
                allKeys[i].setRightCell(allKeys[i+1]);
                allKeys[i].setHourCell(allKeys[i-12]);
            }

        }

        return allKeys;
    }

    public static CamelotClass[] getNeighbouringKeys(CamelotClass key){
        CamelotClass[] adjacentKeys = new CamelotClass[3];
        adjacentKeys[0] = key.leftCell;
        adjacentKeys[1] = key.rightCell;
        adjacentKeys[2] = key.hourCell;
        return adjacentKeys;
    }

    public static CamelotClass getKeyFromString(CamelotClass[] keys, String key){

        int index = -1;
        switch (key){
            case "1A":
                index = 0;
                break;
            case "2A":
                index = 1;
                break;
            case "3A":
                index = 2;
                break;
            case "4A":
                index = 3;
                break;
            case "5A":
                index = 4;
                break;
            case "6A":
                index = 5;
                break;
            case "7A":
                index = 6;
                break;
            case "8A":
                index = 7;
                break;
            case "9A":
                index = 8;
                break;
            case "10A":
                index = 9;
                break;
            case "11A":
                index = 10;
                break;
            case "12A":
                index = 11;
                break;
            case "1B":
                index = 12;
                break;
            case "2B":
                index = 13;
                break;
            case "3B":
                index = 14;
                break;
            case "4B":
                index = 15;
                break;
            case "5B":
                index = 16;
                break;
            case "6B":
                index = 17;
                break;
            case "7B":
                index = 18;
                break;
            case "8B":
                index = 19;
                break;
            case "9B":
                index = 20;
                break;
            case "10B":
                index = 21;
                break;
            case "11B":
                index = 22;
                break;
            case "12B":
                index = 23;
                break;


        }

        return keys[index];


    }



}
