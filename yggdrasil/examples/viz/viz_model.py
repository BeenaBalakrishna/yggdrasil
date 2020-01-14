# Import classes for input/output channels
import matplotlib.pyplot as plt
from yggdrasil.interface.YggInterface import YggInput

# Initialize input/output channels
in_channel = YggInput('input_viz')

xs = []
ys = []

plot_num = 0
with open("something.csv", "w") as f:
    f.write("x,y\n")
# Loop until there is no longer input or the queues are closed
while True:

    # Receive input from input channel
    # If there is an error, the flag will be False
    flag, val = in_channel.recv()
    if not flag:
        print("Model B: No more input.")
        # Things that should be done at the end can go here
        break

    x, y = val 
    xs.append(x)
    ys.append(y)
    # Print received message
    print('Viz model received blah blah %f, %f' % (x, y))
    
    with open("something.csv", "a") as f:
        f.write("%f, %f\n" % (x,y))
     
    #Plotting simple X-Y graphs
    
    plt.plot(xs, ys)
    plt.xlabel('Input')
    plt.ylabel('Output')
    plt.title('Viz testing')
    plt.savefig('plot1_{}.png'.format(plot_num))
    plot_num += 1
    
    # Any actions that should be taken when a new value is
    # received should go here
