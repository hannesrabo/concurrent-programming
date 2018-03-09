package space;

public class SpaceStation {

    private int fuelNitrogen = 0;
    private int fuelQuantum = 0;
    private int availablePositions;

    private int maxFuelNitrogen;
    private int maxFuelQuantum;

    SpaceStation(int fuelNitrogen, int fuelQuantum, int availablePositions) {
        this.maxFuelNitrogen = fuelNitrogen;
        this.maxFuelQuantum = fuelQuantum;
        this.availablePositions = availablePositions;
    }

    @Override
    public String toString() {
        return "SS: Current: (N:" + fuelNitrogen + " Q:" + fuelQuantum + " P:" + availablePositions + ") Max: [N:"
                + maxFuelNitrogen + " Q:" + maxFuelQuantum + "]";
    }

    public void transferreFuelDelay() throws InterruptedException {
        Thread.sleep(SpaceGlobals.refuelingWaitingTime);
    }

    public synchronized void reserveFuelAndPosition(SpaceVehicle sv, int fuelNitrogen, int fuelQuantum)
            throws InterruptedException {
        while (!(this.availablePositions > 0 && this.fuelNitrogen >= fuelNitrogen && this.fuelQuantum >= fuelQuantum)) {
            System.out.println("Queued: " + sv);
            wait();
        }
        this.fuelNitrogen -= fuelNitrogen;
        this.fuelQuantum -= fuelQuantum;
        availablePositions--;

        System.out.println("R: " + sv);
        System.out.println(this);
    }

    public synchronized void reserveFuel(SpaceVehicle sv, int fuelNitrogen, int fuelQuantum)
            throws InterruptedException {
        while (!(this.fuelNitrogen >= fuelNitrogen && this.fuelQuantum >= fuelQuantum)) {
            System.out.println("Queued: " + sv);
            wait();
        }

        this.fuelNitrogen -= fuelNitrogen;
        this.fuelQuantum -= fuelQuantum;

        System.out.println("RF: " + sv);
        System.out.println(this);
    }

    public synchronized void reserveAddFuelAndPosition(SpaceVehicle sv, int fuelNitrogen, int fuelQuantum)
            throws InterruptedException {
        while (!(availablePositions > 0 && this.fuelNitrogen + fuelNitrogen <= maxFuelNitrogen
                && this.fuelQuantum + fuelQuantum <= maxFuelQuantum)) {
            System.out.println("Queued: " + sv);
            wait();
        }

        this.fuelNitrogen += fuelNitrogen;
        this.fuelQuantum += fuelQuantum;
        availablePositions--;

        System.out.println("F: " + sv);
        System.out.println(this);
        System.out.println("----------------------------------------------");

        // At this point we want more people to go and refuel their ships
        notifyAll();

    }

    public synchronized void leaveStation(SpaceVehicle sv) {
        availablePositions++;

        System.out.println("L: " + sv);
        System.out.println(this);

        notifyAll();
    }

}