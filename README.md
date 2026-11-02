# GameEngine 1.0

Visualizador de geometría 3D desarrollado en C++ para la **Entrega 1 de Motores Gráficos**. Permite cargar modelos FBX y texturas, explorar una escena con cámara libre y orbital, inspeccionar sus componentes y transformar objetos desde un editor basado en Dear ImGui.

El renderer utiliza **OpenGL 3.3 y shaders integrados**. La escena inicial carga Baker House, un coche y un sillón, con sus texturas difusas.

**Repositorio:** [Alejandrorg5/GameEngine](https://github.com/Alejandrorg5/GameEngine)

## Integrantes

| Integrante | GitHub |
| --- | --- |
| Hugo Escobar — Hugg_ESC | [HuggoESC](https://github.com/HuggoESC) |
| Alejandro Rodriguez — Alejandrorg5 | [Alejandrorg5](https://github.com/Alejandrorg5) |

## Ejecución

Requisitos: **Windows 10/11 de 64 bits**, tarjeta gráfica y controlador compatibles con **OpenGL 3.3 o superior**.

1. Descarga el paquete compilado de la entrega desde [Releases](https://github.com/Alejandrorg5/GameEngine/releases), cuando esté publicado. El ZIP de código fuente de GitHub no sustituye al paquete ejecutable.
2. Extrae el ZIP completo antes de abrirlo.
3. Ejecuta `GameEngine.exe`, conservando junto a él las DLL, `config.xml` y la carpeta `Assets`. El paquete incluye los runtimes de Visual C++ necesarios.

La versión compilada no requiere Visual Studio ni vcpkg. Si aparece un error de OpenGL, comprueba la compatibilidad de la GPU y su controlador. Los errores de importación se muestran en **Console**.

## Controles

Los controles de cámara con ratón se utilizan sobre **Scene**. Para los atajos de teclado, sitúa el cursor sobre Scene o dale el foco; no se activan mientras editas un campo de texto. LMB y RMB significan botón izquierdo y derecho del ratón.

| Acción | Control |
| --- | --- |
| Seleccionar un modelo completo | Clic izquierdo sobre su geometría en Scene. |
| Quitar la selección | Clic izquierdo sobre una zona vacía de Scene. |
| Seleccionar una malla o nodo concreto | Clic en su nombre en Hierarchy; las flechas despliegan sus hijos. |
| Mirar libremente | Mantener RMB y mover el ratón. |
| Avanzar, retroceder y desplazarse lateralmente | Mantener RMB y usar W, S, A y D. |
| Duplicar la velocidad de desplazamiento | Mantener Shift durante RMB + WASD. |
| Orbitar la selección | Mantener Alt + LMB y arrastrar. |
| Acercar o alejar la cámara | Rueda del ratón. |
| Enfocar el objeto seleccionado | F. |
| Activar el gizmo de movimiento | W o botón **Move (W)**. |
| Activar el gizmo de rotación | E o botón **Rotate (E)**. |
| Activar el gizmo de escala | R o botón **Scale (R)**. |
| Transformar con el gizmo | Arrastrar sus ejes, planos o anillos con LMB; el centro de escala permite escalar uniformemente. |
| Cambiar el espacio de transformación | Botones **Local / World** para movimiento y rotación. La escala utiliza ejes locales. |
| Modificar valores numéricos | Editar **Position**, **Rotation** y **Scale** en Inspector. |
| Renombrar un objeto | Editar **Name** en Inspector. |
| Eliminar un objeto | **Delete selected** en Hierarchy, menú contextual del nodo → **Delete**, o Supr con foco en Hierarchy/Scene. |
| Cancelar la confirmación de eliminación | **Cancel** o Escape. |
| Salir | **File → Exit** o cerrar la ventana. |

Con RMB pulsado, W controla el desplazamiento y no cambia de gizmo. Alt + LMB queda reservado para orbitar. Al arrastrar un gizmo se mantiene la selección y se bloquea el control de cámara durante la manipulación.

Eliminar un objeto con hijos solicita confirmación y elimina toda su rama de la escena. **No borra los modelos ni las texturas del disco.** La raíz de la escena no se puede eliminar.

## Importar modelos y texturas

- Arrastra un archivo **FBX** desde el explorador a la ventana del motor. Se añade a la escena conservando los modelos existentes y se selecciona el modelo importado.
- Las texturas difusas referenciadas por el FBX se cargan desde sus rutas externas o desde los datos embebidos. Conserva los archivos de textura y sus rutas relativas al mover un modelo.
- Para sustituir una textura, selecciona un objeto y arrastra un archivo **PNG o DDS** a la ventana del motor. Si seleccionas el padre de un modelo, la textura se aplica también a sus mallas hijas.
- Un archivo incompatible o que no se pueda cargar produce un mensaje en Console. No reemplaza la escena existente.
- Utiliza **Create → Cube / Sphere / Pyramid / Cylinder** para añadir geometrías básicas.

## Ventanas y opciones del editor

**Scene** muestra la escena 3D, el cuadro de selección y los gizmos. **Hierarchy** muestra los GameObjects y sus relaciones padre-hijo. **Inspector** muestra los componentes del objeto seleccionado:

- **Transform:** posición, rotación y escala editables; la rotación se expresa en grados y las transformaciones son relativas al padre.
- **Mesh:** cantidad de vértices, índices y triángulos; opciones **Vertex normals**, **Face normals** y **Normal length** para visualizar normales y ajustar su longitud.
- **Texture:** ruta y dimensiones; **Checker texture** alterna la textura de cuadrados blancos y negros, también en las mallas hijas del objeto seleccionado.

Los nodos que agrupan un modelo pueden no contener geometría directamente. Selecciona una malla hija en Hierarchy para consultar sus datos y normales.

**Console** registra la inicialización, el flujo de la aplicación, las importaciones de Assimp y DevIL y los errores. El botón **Clear** vacía el registro visible.

**Configuration** permite modificar:

| Grupo | Opciones |
| --- | --- |
| Engine | Gráfico de FPS y límite de FPS entre 0 y 300; 0 significa sin límite. |
| Renderer | VSync, prueba de profundidad, wireframe, cuadrícula y color de fondo. |
| Window | Tamaño de ventana —confirmar con Enter—, pantalla completa, bordes y posibilidad de redimensionar. |
| Input / Camera | Velocidad de movimiento, sensibilidad de la vista y velocidad de zoom. |
| Hardware / Memory / Libraries | Memoria del proceso y del sistema, núcleos lógicos, GPU y versiones de OpenGL, SDL, DevIL, Assimp y Dear ImGui. |

La barra superior incluye:

- **File:** salir de la aplicación.
- **View:** mostrar u ocultar Scene, Hierarchy, Inspector, Console, Configuration y About. Permite recuperar una ventana cerrada con su botón X.
- **Create:** crear las cuatro primitivas disponibles.
- **Help → Documentation:** abre la ruta `docs` del repositorio. Esta ruta requiere que la documentación esté publicada allí.
- **Help → Report a bug:** abre [Issues](https://github.com/Alejandrorg5/GameEngine/issues).
- **Help → Latest release:** abre [Releases](https://github.com/Alejandrorg5/GameEngine/releases).
- **Help → About:** muestra el nombre y versión del motor, los integrantes, enlaces de GitHub, bibliotecas, créditos de modelos y el texto de licencia mostrado por la aplicación.

## Funcionalidades extra

- **Edición de Transform en Inspector**, además de la consulta exigida por E1.
- **Selección precisa con el ratón:** rayo de cámara, descarte mediante AABB e intersección con triángulos para escoger el impacto más cercano. Un clic en una parte del FBX selecciona el modelo completo.
- **Resaltado de selección** y sincronización entre Scene, Hierarchy e Inspector.
- **Gizmos de movimiento, rotación y escala con ImGuizmo**, atajos W/E/R y modos Local/World donde corresponde.
- **Eliminación de objetos y subárboles**, con confirmación cuando hay hijos y conservación de los archivos originales.
- **Importación aditiva** de varios FBX y aplicación de texturas a las mallas de un modelo desde su nodo padre.
- **Compatibilidad OpenGL 3.3**, carga de recursos con rutas Unicode y empaquetado portable para Windows x64.

## Observaciones para la evaluación

- Los GameObjects de malla disponen de Transform, Mesh y Textura. Se conservan nodos intermedios del FBX para representar su jerarquía y transformaciones.
- La interfaz se implementa en `Modules/Editor.*`; la gestión de escena, el renderer, los cargadores y los componentes mantienen sus responsabilidades fuera del editor.
- Baker House se ajusta al cargar para aparecer de pie y con una escala adecuada, sin modificar el FBX original.
- Las transformaciones y eliminaciones son cambios de la sesión. Esta entrega **no incluye guardado/carga de escenas ni deshacer/rehacer**.
- Una rotación bajo un padre con escala no uniforme puede requerir una deformación que Transform no representa. En ese caso se rechaza la operación y se muestra un aviso para seleccionar el padre.
- La distribución se ha probado localmente desde una carpeta independiente; su ejecución en otro equipo requiere cumplir los requisitos de Windows x64 y OpenGL indicados.
- El documento de reparto de tareas y `LICENCE.md` se gestionan como documentos separados de este README.

## Compilación desde el código fuente

Para desarrollo se necesitan CMake 3.21 o superior, Ninja, un compilador MSVC con soporte C++17, Windows SDK y vcpkg. Define `VCPKG_ROOT` apuntando a la instalación de vcpkg y ejecuta desde una terminal de desarrollo x64 de Visual Studio:

```powershell
cmake --preset vcpkg
cmake --build --preset release
cpack --config build/CPackConfig.cmake -C Release
```

El manifiesto de vcpkg declara SDL3, GLAD, GLM, Dear ImGui, ImGuizmo, Assimp, DevIL y pugixml. El paquete generado se llama `GameEngine-1.0.0-Windows-x64.zip` y se encuentra en `build`. Para preparar la entrega completa, incorpora también el README, la licencia del proyecto y el documento de tareas; comprueba el límite de 30 MB del ZIP final.

## Recursos y créditos

- **Baker House:** modelo proporcionado para la asignatura.
- **Coche:** Kenney Car Kit, de [Kenney](https://kenney.nl/assets/car-kit), CC0.
- **Sillón:** KayKit Furniture Bits, de Kay Lousberg, [repositorio oficial](https://github.com/KayKit-Game-Assets/KayKit-Furniture-Bits-1.0), CC0.
- Las licencias de las bibliotecas distribuidas se incluyen en la carpeta `Licenses` del paquete.
