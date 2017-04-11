Alt-H1 RayTracer rendering picture

![alt text](https://github.com/lidaiqing/RayTracer/blob/master/dragon_4096.jpg "Rendered Picture")


This project implements techniques like antialiasing, area light for soft shading and a scene designed by ourselves.

We further implemented texture mapping, environmental mapping, refraction, handling arbitrary surface mesh and multi-threading.

We started reading the starter code so we could understand the purpose of each function quickly and correctly. Also, we discussed within the group before writing the code to check we reach an agreement on how we transfer theoretical knowledge into practical works.

The basic ideas of this project is straightforward but the hard part is not to make tiny mistakes and improve the performance of rendering. So working on each functionality requires extra cautious and crystal clear about what we are doing.

The project helped us have an overview of how ray trace is implemented in industry and gives me an idea that the world to model transformation can greatly improve the performance and reduce the computation complexity.

The biggest challenge we faced were trying to render a “dragon” mash containing about 2 million triangles and thus there is no way to treat each triangle as a separate object in “findFIrsthit” To improve the performance of our rendering, we imported open source bounding box algorithm to make rendering dragon possible. The basic idea is we group triangle into hierarchy clusters and testing intersection between rays and those clusters instead.

