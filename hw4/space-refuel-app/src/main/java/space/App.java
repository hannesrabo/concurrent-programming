package space;

/**
 * Hello world!
 *
 */
public class App {
    public static void main(String[] args) {

        int nrSpaceVehicles = 20;
        int nrFuelVehicles = 4;
        int nrFuelingPositions = 4;

        SpaceStation st = new SpaceStation(100000, 50000, nrFuelingPositions);

        System.out.println("Space Station Created:");
        System.out.println(st);
        System.out.println("-----------------------------");

        SpaceVehicle[] spaceVehicles = new SpaceVehicle[nrSpaceVehicles + nrFuelVehicles];

        for (int i = 0; i < spaceVehicles.length; i++) {
            if (i < nrSpaceVehicles) {
                spaceVehicles[i] = new SpaceVehicle(100, 60, st);

            } else {
                spaceVehicles[i] = new SpaceFuelVehicle(300, 150, st, 14000, 7000);
            }

            spaceVehicles[i].id = i;

            spaceVehicles[i].start();

            System.out.println("Started: " + spaceVehicles[i]);
        }

        for (int i = 0; i < spaceVehicles.length; i++) {
            if (i < nrSpaceVehicles) {
                try {
                    spaceVehicles[i].join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

            } else {
                break;
            }
        }

        System.out.println("Program finished");
        System.exit(0);
    }
}
