from dataclasses import dataclass
import pybullet as p
import time
from scipy import signal
import numpy as np
import utils
import argparse


@dataclass
class Point:
    x: float
    y: float

@dataclass
class World:
    """A class to hold the world objects"""
    plate: int
    sphere: int


state = {
    "old_x"   :  0.0 ,
    "old_y"   :  0.0 ,
    "old_ex"  :  0.0 ,
    "old_ey"  :  0.0 ,
    "x_raw"     :  [ ] ,    
    "y_raw"     :  [ ]    

    
    
}


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--setpoint", type=float, nargs=2, default=(0.0,0.0))
    parser.add_argument("--kp", type=float, default=0.0) # set these values to tune the PD controller
    parser.add_argument("--kd", type=float, default=0.0) # set these values to tune the PD controller
    parser.add_argument("--noise", action="store_true", help="Add noise to the measurements")
    parser.add_argument("--filtered", action="store_true", help="filter the measurements")
    cmd_args = parser.parse_args()
    print(cmd_args)
    cmd_args.setpoint = Point(*cmd_args.setpoint)
    return cmd_args




def run_controller(kp, kd, setpoint, noise, filtered, world: World):

    def set_plate_angles(theta_x, theta_y):
        p.setJointMotorControl2(world.plate, 1, p.POSITION_CONTROL, targetPosition=np.clip(theta_x, -0.9, 0.9), force=5, maxVelocity=2)
        p.setJointMotorControl2(world.plate, 0, p.POSITION_CONTROL, targetPosition=np.clip(-theta_y, -0.9, 0.9), force=5, maxVelocity=2)

    # you can set the variables that should stay accross control loop here



    def pd_controller(x, y, kp, kd, setpoint):
        """Implement a PD controller, you can access the setpoint via setpoint.x and setpoint.y
        the plate is small around 0.1 to 0.2 meters. You will have to calculate the error and change in error and 
        use those to calculate the angle to apply to the plate."""

        dt = 0.01 # Everything runs every 10ms, so thats 100hz
        
        ex = x - setpoint.x
        ey = y - setpoint.y
        dex = (ex - state["old_ex"]) / dt # derivative of change in pos, which is just velocity
        dyx = (ey - state["old_ey"]) / dt

#        vx, vy, _ = p.getBaseVelocity(world.sphere)[0]

        out_x = -kp * ex - kd * dex # equation striaght ripped from slides.
        out_y = -kp * ey - kd * dyx

        state["old_x"] = x
        state["old_y"] = y
        state["old_ex"] = ex
        state["old_ey"] = ey

        return out_x , out_y


    def filter_val( val ):
        """Implement a filter here, you can use scipy.signal.butter to compute the filter coefficients and then scipy.signal.lfilter to apply the filter.but we recommend you implement it yourself instead of using lfilter because you'll have to do that on the real system later.
        Take a look at the butterworth example written by Renato for inspiration."""


    # SAmpling rate Hz
        fc = 100


# Butterworth Filter Design ###

# Set the filter order
        N_ord = 4

# Set the filter cutoff in Hz
        f_cutoff = 13

# Compute percentage w.r.t. Nyquist frequency
        wn = f_cutoff * 2/fc


# Get the signal coefficients
        b , a = signal.butter( N_ord , wn , 'low' , analog = False )

# return vector of filtered


        return signal.lfilter( b , a , val )


    def every_10ms(i: int, t: float):
        '''This function is called every ms and performs the following:
        1. Get the measurement of the position of the ball
        2. Calculate the forces to be applied to the plate
        3. Apply the forces to the plate
        '''
        (x,y,z), orientation = p.getBasePositionAndOrientation(world.sphere)
        if noise:
            x += utils.noise(t) # the noise added has a frequency between 30 and 50 Hz
            y += utils.noise(t, seed = 43) # so that the noise on y is different than the one on x
       
        
        if filtered:
            state["x_raw"] = np.concatenate( ( state["x_raw"] , [ x ]  ) )
            state["y_raw"] = np.concatenate( ( state["y_raw"] , [ x ]  ) )
            x_filt = filter_val( state["x_raw"] )
            y_filt = filter_val( state["y_raw"] )
            x = x_filt[ -1 ]
            y = y_filt[ -1 ]


        (angle_x, angle_y) = pd_controller(x, y, kp, kd, setpoint)
        set_plate_angles(angle_x, angle_y)

        if i%10 == 0: # print every 100 ms
            print(f"t: {t:.2f}, x: {x:.3f},\ty: {y:.3f},\tax: {angle_x:.3f},\tay: {angle_y:.3f}")

    utils.loop_every(0.01, every_10ms) # we run our controller at 100 Hz using a linux alarm signal

def run_simulation( initial_ball_position = Point(np.random.uniform(-0.2, 0.2),
                                                  np.random.uniform(-0.2, 0.2))):
    p.connect(p.GUI)
    p.setAdditionalSearchPath("assets")
    plate = p.loadURDF("plate.urdf")
    sphere = p.createMultiBody(0.2
        , p.createCollisionShape(p.GEOM_SPHERE, radius=0.04)
        , basePosition = [initial_ball_position.x,initial_ball_position.y,0.5]
    )

    #zoom to the plate
    p.resetDebugVisualizerCamera(cameraDistance=1.0, cameraYaw=0, cameraPitch=-45, cameraTargetPosition=[0,0,0])

    p.setJointMotorControl2(plate, 0, p.POSITION_CONTROL, targetPosition=0, force=5, maxVelocity=2)
    p.setJointMotorControl2(plate, 1, p.POSITION_CONTROL, targetPosition=0, force=5, maxVelocity=2)

    p.setGravity(0, 0, -9.8)

    #update the simulation at 100 Hz
    p.setTimeStep(0.01)
    p.setRealTimeSimulation(1)
    return World(plate=plate, sphere=sphere)



if __name__ == "__main__":
    cmd_args = parse_args()
    world = run_simulation()
    run_controller(**vars(cmd_args), world=world)
    time.sleep(10000)
