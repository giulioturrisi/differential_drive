from math import sin, cos, pi
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile
from geometry_msgs.msg import Quaternion
from sensor_msgs.msg import JointState
from tf2_ros import TransformBroadcaster, TransformStamped

from geometry_msgs.msg import TwistStamped

class StatePublisher(Node):

    def __init__(self):
        #rclpy.init()
        super().__init__('state_publisher')

        qos_profile = QoSProfile(depth=10)
        self.joint_pub = self.create_publisher(JointState, 'joint_states', qos_profile)
        self.broadcaster = TransformBroadcaster(self, qos=qos_profile)
        self.nodeName = self.get_name()
        self.get_logger().info("{0} started".format(self.nodeName))

        self.subscription_odom = self.create_subscription(TwistStamped, 'odometry', self.odometry_callback,1)
        self.create_timer(0.005, self.tf_callback)

        self.last_time = 0.0

        self.odom_x = 0.0
        self.odom_y = 0.0
        self.odom_theta = 0.0



    def tf_callback(self):
        # message declarations
        odom_trans = TransformStamped()
        odom_trans.header.frame_id = 'odom'
        odom_trans.child_frame_id = 'base_footprint'
        joint_state = JointState()
        # update joint_state
        now = self.get_clock().now()
        joint_state.header.stamp = now.to_msg()
        joint_state.name = ['joint_left', 'joint_right']
        joint_state.position = [0.0, 0.0]

        # update transform
        odom_trans.header.stamp = now.to_msg()
        odom_trans.transform.translation.x = self.odom_x #self.odom_x
        odom_trans.transform.translation.y = self.odom_y #self.odom_y
        odom_trans.transform.translation.z = 0.05 
        odom_trans.transform.rotation = \
            euler_to_quaternion(0, 0, self.odom_theta) # roll,pitch,yaw #self.odom_theta

        # send the joint state and transform
        self.joint_pub.publish(joint_state)
        #self.broadcaster.sendTransform(odom_trans)

       

    def odometry_callback(self, msg):
        #print("odometry received")

        v_reconstructed = msg.twist.linear.x 
        w_reconstructed = msg.twist.angular.z 
        time = msg.header.stamp.nanosec/1000000000.00

        if(self.last_time != 0.0):
            Ts = time - self.last_time    
            if(Ts > 0.0):
                print("Ts", Ts)
                self.odom_x = self.odom_x + v_reconstructed*Ts*cos(self.odom_theta);
                self.odom_y = self.odom_y + v_reconstructed*Ts*sin(self.odom_theta);
                self.odom_theta = self.odom_theta + w_reconstructed*Ts

        self.last_time = time


def euler_to_quaternion(roll, pitch, yaw):
    qx = sin(roll/2) * cos(pitch/2) * cos(yaw/2) - cos(roll/2) * sin(pitch/2) * sin(yaw/2)
    qy = cos(roll/2) * sin(pitch/2) * cos(yaw/2) + sin(roll/2) * cos(pitch/2) * sin(yaw/2)
    qz = cos(roll/2) * cos(pitch/2) * sin(yaw/2) - sin(roll/2) * sin(pitch/2) * cos(yaw/2)
    qw = cos(roll/2) * cos(pitch/2) * cos(yaw/2) + sin(roll/2) * sin(pitch/2) * sin(yaw/2)
    return Quaternion(x=qx, y=qy, z=qz, w=qw)

def main():
    rclpy.init()
    node = StatePublisher()

    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()