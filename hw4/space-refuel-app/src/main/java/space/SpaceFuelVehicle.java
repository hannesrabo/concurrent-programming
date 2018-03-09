package space;

public class SpaceFuelVehicle extends SpaceVehicle {

    public int storageBufferNitrogen;
    public int storageBufferQuantum;

    SpaceFuelVehicle(int nitrogenTank, int quantumTank, SpaceStation spaceStation, int bufferNitrogen,
            int bufferQuantum) {
        super(nitrogenTank, quantumTank, spaceStation);

        this.storageBufferNitrogen = bufferNitrogen;
        this.storageBufferQuantum = bufferQuantum;
    }

    @Override
    public String toString() {
        return "SFV<" + id + "> (N:" + fuelTankNitrogen + " Q:" + fuelTankQuantum + " BN:" + storageBufferNitrogen
                + " BQ:" + storageBufferQuantum + "]";
    }

    @Override
    public void run() {
        int iteration = 0;
        while (iteration < SpaceGlobals.nrIterations) {
            try {
                Thread.sleep(SpaceGlobals.getRandomFuelVehicleTime());
            } catch (InterruptedException e) {
                e.printStackTrace();
                return;
            }

            try {
                // Get a position when ther is enough room in the space station
                spaceStation.reserveAddFuelAndPosition(this, storageBufferNitrogen, storageBufferQuantum);

                // Check for fuel (we already have a position)
                // There should always be enough fuel right now!
                spaceStation.reserveFuel(this, fuelTankNitrogen, fuelTankQuantum);

                spaceStation.transferreFuelDelay(); // Not synchronized!                
            } catch (InterruptedException e) {
                e.printStackTrace();
                return;
            }
            // We have reserved the resources from space station

            spaceStation.leaveStation(this);

            iteration++;
        }
    }
}