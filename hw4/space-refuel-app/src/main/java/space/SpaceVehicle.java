package space;

public class SpaceVehicle extends Thread {

    public int id;
    public int fuelTankNitrogen;
    public int fuelTankQuantum;
    protected SpaceStation spaceStation;

    SpaceVehicle(int nitrogenTank, int quantumTank, SpaceStation spaceStation) {
        this.fuelTankNitrogen = nitrogenTank;
        this.fuelTankQuantum = quantumTank;
        this.spaceStation = spaceStation;
    }

    @Override
    public String toString() {
        return "SV<" + id + "> (N:" + fuelTankNitrogen + " Q:" + fuelTankQuantum + ")";
    }

    @Override
    public void run() {
        int iteration = 0;
        while (iteration < SpaceGlobals.nrIterations) {
            try {
                Thread.sleep(SpaceGlobals.getRandomSpaceVehicleTime());
            } catch (InterruptedException e) {
                e.printStackTrace();
                return;
            }

            // Check for position and fuel
            try {
                spaceStation.reserveFuelAndPosition(this, fuelTankNitrogen, fuelTankQuantum);

                spaceStation.transferreFuelDelay(); // Not synchronized!
            } catch (InterruptedException e) {
                e.printStackTrace();
                return;
            }

            spaceStation.leaveStation(this);

            iteration++;
        }
    }
}