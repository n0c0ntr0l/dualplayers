package com.players.jason.dualplayers.FileProcessing;

/**
 * Created by Jason on 26/03/2015.
 */
public class WavFileException extends Exception
{
    public WavFileException()
    {
        super();
    }

    public WavFileException(String message)
    {
        super(message);
    }

    public WavFileException(String message, Throwable cause)
    {
        super(message, cause);
    }

    public WavFileException(Throwable cause)
    {
        super(cause);
    }
}