package space;

import java.util.Random;

public class SpaceGlobals {
    public static final int nrIterations = 20;

    public static final int minWaitTimeSpaceVehicle = 2000;
    public static final int minWaitTimeFuelVehicle = 5000;
    public static final int variableWaitTimeSpaceVehicle = 1000;
    public static final int variableWaitTimeFuelVehicle = 1000;

    public static final int refuelingWaitingTime = 20;

    public static Random rand = new Random();

    public static int getRandomSpaceVehicleTime() {
        return minWaitTimeSpaceVehicle + (int) (rand.nextDouble() * variableWaitTimeSpaceVehicle);
    }

    public static int getRandomFuelVehicleTime() {
        return minWaitTimeFuelVehicle + (int) (rand.nextDouble() * variableWaitTimeFuelVehicle);
    }
}