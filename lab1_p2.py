import matplotlib.pyplot as plt

# Load data
orig = [tuple(map(float, line.split())) for line in open("delays_original_topology.txt")]
mod  = [tuple(map(float, line.split())) for line in open("delays_modified_topology.txt")]

x1, y1 = zip(*orig)
x2, y2 = zip(*mod)

plt.plot(x1, y1, 'o-', label="Topologia Original")
plt.plot(x2, y2, 's-', label="Topologia Modificada")

plt.xlabel("Número do Pacote")
plt.ylabel("Atraso Ponto-a-ponto (s)")
plt.title("Atraso Ponto-a-ponto vs. Número do Pacote")
plt.legend()
plt.grid(True)
plt.show()
