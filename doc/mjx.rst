================
MuJoCo XLA (MJX)
================

Starting with version 3.0.0, MuJoCo includes MuJoCo XLA (MJX) under the
`mjx <https://github.com/google-deepmind/mujoco/tree/main/mjx>`__ directory.  MJX allows MuJoCo to run on compute
hardware supported by the `XLA <https://www.tensorflow.org/xla>`__ compiler via the
`JAX <https://github.com/google/jax#readme>`__ framework.  MJX runs on a
`all platforms supported by JAX <https://jax.readthedocs.io/en/latest/installation.html#supported-platforms>`__: Nvidia
and AMD GPUs, Apple Silicon, and `Google Cloud TPUs <https://cloud.google.com/tpu>`__.

The MJX API is consistent with the main simulation functions in the MuJoCo API, although it is currently missing some
features. While the :ref:`API documentation <Mainsimulation>` is applicable to both libraries, we indicate features
unsupported by MJX in the :ref:`notes <MjxFeatureParity>` below.

MJX is distributed as a separate package called ``mujoco-mjx`` on `PyPI <https://pypi.org/project/mujoco-mjx>`__.
Although it depends on the main ``mujoco`` package for model compilation and visualization, it is a re-implementation of
MuJoCo that uses the same algorithms as the MuJoCo implementation. However, in order to properly leverage JAX, MJX
deliberately diverges from the MuJoCo API in a few places, see below.

MJX is a successor to the `generalized physics pipeline <https://github.com/google/brax/tree/main/brax/generalized>`__
in Google's `Brax <https://github.com/google/brax>`__ physics and reinforcement learning library.  MJX was built
by core contributors to both MuJoCo and Brax, who will together continue to support both Brax (for its reinforcement
learning algorithms and included environments) and MJX (for its physics algorithms).  A future version of Brax will
depend on the ``mujoco-mjx`` package, and Brax's existing
`generalized pipeline <https://github.com/google/brax/tree/main/brax/generalized>`__ will be deprecated.  This change
will be largely transparent to users of Brax.

.. _MjxNotebook:

Tutorial notebook
=================

The following IPython notebook demonstrates the use of MJX along with reinforcement learning to train humanoid and
quadruped robots to locomote: |colab|.

.. |colab| image:: https://colab.research.google.com/assets/colab-badge.svg
           :target: https://colab.research.google.com/github/google-deepmind/mujoco/blob/main/mjx/tutorial.ipynb

.. _MjxInstallation:

Installation
============

The recommended way to install this package is via `PyPI <https://pypi.org/project/mujoco-mjx/>`__:

.. code-block:: shell

   pip install mujoco-mjx

A copy of the MuJoCo library is provided as part of this package's depdendencies and does **not** need to be downloaded
or installed separately.

.. _MjxUsage:

Basic usage
===========

Once installed, the package can be imported via ``from mujoco import mjx``. Structs, functions, and enums are available
directly from the top-level ``mjx`` module.

.. _MjxStructs:

Structs
-------

Before running MJX functions on an accelerator device, structs must be copied onto the device via the ``mjx.device_put``
function.  Placing an :ref:`mjModel` on device yields an ``mjx.Model``.  Placing an :ref:`mjData` on device yields
an ``mjx.Data``:

.. code-block:: python

   model = mujoco.MjModel.from_xml_string("...")
   data = mujoco.MjData(model)
   mjx_model = mjx.device_put(model)
   mjx_data = mjx.device_put(data)

These MJX variants mirror their MuJoCo counterparts but have three key differences:

#. Fields in ``mjx.Model`` and ``mjx.Data`` are JAX arrays copied onto device, instead of numpy arrays.
#. Some fields are missing from ``mjx.Model`` and ``mjx.Data`` for features that are
   :ref:`unsupported <mjxFeatureParity>` in MJX.
#. Arrays in ``mjx.Model`` and ``mjx.Data`` support adding batch dimensions. Batch dimensions are a natural way to
   express domain randomization (in the case of ``mjx.Model``) or high-throughput simulation for reinforcement learning
   (in the case of ``mjx.Data``).


Neither ``mjx.Model`` nor ``mjx.Data`` are meant to be constructed manually.  An ``mjx.Data`` may be created by calling
``mjx.make_data``, which mirrors the :ref:`mj_makeData` function in MuJoCo:

.. code-block:: python

   model = mujoco.MjModel.from_xml_string("...")
   mjx_model = mjx.device_put(model)
   mjx_data = mjx.make_data(model)

Using ``mjx.make_data`` may be preferable when constructing batched ``mjx.Data`` structures inside of a ``vmap``.

.. _MjxFunctions:

Functions
---------

MuJoCo functions are exposed as MJX functions of the same name, but following
`PEP 8 <https://peps.python.org/pep-0008/>`__-compliant names.  Most of the :ref:`main simulation <Mainsimulation>` and
some of the :ref:`sub-components <Subcomponents>` for forward simulation are available from the top-level ``mjx`` module.

MJX functions are not `JIT compiled <https://jax.readthedocs.io/en/latest/jax-101/02-jitting.html>`__ by default -- we
leave it to the user to JIT MJX functions, or JIT their own functions that reference MJX functions.  See the
:ref:`minimal example <MjxExample>` below.

.. _MjxEnums:

Enums and constants
-------------------

MJX enums are available as ``mjx.EnumType.ENUM_VALUE``, for example ``mjx.JointType.FREE``. Enums for unsupported MJX
features are omitted from the MJX enum declaration.  MJX declares no constants but references MuJoCo constants directly.

.. _MjxExample:

Minimal example
---------------

.. code-block:: python

  # Throw a ball at 100 different velocities.

   import jax
   import mujoco
   from mujoco import mjx

   XML=r"""
   <mujoco>
     <worldbody>
       <body>
         <freejoint/>
         <geom size=".15" mass="1" type="sphere"/>
       </body>
     </worldbody>
   </mujoco>
   """

   model = mujoco.MjModel.from_xml_string(XML)
   mjx_model = mjx.device_put(model)

   @jax.vmap
   def batched_step(vel):
     mjx_data = mjx.make_data(mjx_model)
     qvel = mjx_data.qvel.at[0].set(vel)
     mjx_data = mjx_data.replace(qvel=qvel)
     pos = mjx.step(mjx_model, mjx_data).qpos[0]
     return pos

   vel = jax.numpy.arange(0.0, 1.0, 0.01)
   pos = jax.jit(batched_step)(vel)
   print(pos)

.. _MjxFeatureParity:

Feature Parity
==============

MJX supports most of the main simulation features of MuJoCo, with a few exceptions.  MJX will raise an exception if
asked to copy to device an :ref:`mjModel` with field values referencing unsupported features.

The following features are **fully supported** in MJX:

.. list-table::
   :width: 90%
   :align: left
   :widths: 1 5
   :header-rows: 1

   * - Category
     - Feature
   * - Dynamics
     - :ref:`Forward <mj_forward>`
   * - :ref:`Joint <mjtJoint>`
     - ``FREE``, ``BALL``, ``SLIDE``, ``HINGE``
   * - :ref:`Transmission <mjtTrn>`
     - ``TRN_JOINT``
   * - :ref:`Actuation <geactuation>`
     - ``DYN_NONE``, ``DYN_INTEGRATOR``, ``DYN_FILTER``, ``GAIN_FIXED``, ``GAIN_AFFINE``, ``BIAS_NONE``,
       ``BIAS_AFFINE``
   * - :ref:`Geom <mjtGeom>`
     - ``PLANE``, ``SPHERE``, ``CAPSULE``, ``BOX``, ``MESH``
   * - :ref:`Constraint <mjtConstraint>`
     - ``EQUALITY``, ``FRICTION_DOF``, ``LIMIT_JOINT``, ``CONTACT_PYRAMIDAL``
   * - :ref:`Equality <mjtEq>`
     - ``CONNECT``, ``WELD``
   * - :ref:`Integrator <mjtIntegrator>`
     - ``EULER``, ``RK4``
   * - :ref:`Cone <mjtCone>`
     - ``PYRAMIDAL``
   * - :ref:`Condim <coContact>`
     - 3
   * - :ref:`Solver <mjtSolver>`
     - ``CG``
   * - Fluid Model
     - :ref:`flInertia`

The following features are **in development** and coming soon:

.. list-table::
   :width: 90%
   :align: left
   :widths: 1 5
   :header-rows: 1

   * - Category
     - Feature
   * - Dynamics
     - :ref:`Inverse <mj_inverse>`
   * - :ref:`Transmission <mjtTrn>`
     - ``TRN_TENDON``
   * - :ref:`Geom <mjtGeom>`
     - ``HFIELD``, ``ELLIPSOID``, ``CYLINDER``, ``SDF``
   * - :ref:`Integrator <mjtIntegrator>`
     - ``IMPLICIT``, ``IMPLICITFAST``
   * - :ref:`Cone <mjtCone>`
     - ``ELLIPTIC``
   * - :ref:`Condim <coContact>`
     - 1, 4, 6
   * - :ref:`Solver <mjtSolver>`
     - ``NEWTON``
   * - Fluid Model
     - :ref:`flEllipsoid`
   * - :ref:`Tendons <tendon>`
     - :ref:`Spatial <tendon-spatial>`, :ref:`Fixed <tendon-fixed>`
   * - :ref:`Equality <mjtEq>`
     - ``JOINT``, ``TENDON``
   * - :ref:`Sensors <mjtSensor>`
     - All except ``mjSENS_PLUGIN``, ``mjSENS_USER``

The following features are **unsupported**:

.. list-table::
   :width: 90%
   :align: left
   :widths: 1 5
   :header-rows: 1

   * - Category
     - Feature
   * - :ref:`Transmission <mjtTrn>`
     - ``TRN_JOINTINPARENT``, ``TRN_SLIDERCRANK``, ``TRN_SITE``, ``TRN_BODY``, ``MUSCLE``
   * - :ref:`Solver <mjtSolver>`
     - ``PGS``
   * - :ref:`Callbacks <glphysics>`
     - ``mjDYN_USER``, ``mjGAIN_USER``, ``mjBIAS_USER``, ``mjSENS_USER``

.. _MjxSharpBits:

🔪 MJX - The Sharp Bits 🔪
==========================

GPUs and TPUs have unique performance tradeoffs that MJX is subject to.  MJX specializes in simulating big batches of
parallel identical physics scenes using algorithms that can be efficiently vectorized on
`SIMD hardware <https://en.wikipedia.org/wiki/Single_instruction,_multiple_data>`__.  This specialization is useful
for machine learning workloads such as `reinforcement learning <https://en.wikipedia.org/wiki/Reinforcement_learning>`__
that require massive data throughput.

There are certain workflows that MJX is ill-suited for:

Single scene simulation
  Simulating a single scene (1 instance of :ref:`mjData`), MJX can be **10x** slower than MuJoCo, which has been
  carefully optimized for CPU.  MJX works best when simulating thousands or tens of thousands of scenes in parallel.

Large, complex scenes with many contacts
  Accelerators exhibit poor performance for
  `branching code <https://aschrein.github.io/jekyll/update/2019/06/13/whatsup-with-my-branches-on-gpu.html#tldr>`__.
  Branching is used in broad-phase collision detection, when identifying potential collisions between large numbers of
  bodies in a scene.  MJX ships with a simple branchless broad-phase algorithm (see performance tuning) but it is not as
  powerful as the one in MuJoCo.

  To see how this affects simulation, let us consider a physics scene with increasing numbers of physics bodies.  We
  simulate a scene with a variable number of humanoids (from 1 to 10) and then compare MJX's performance on an Nvidia
  A100 GPU to MuJoCo on a 12-core workstation:

  .. figure:: images/mjx/mujoco_vs_mjx_large_scene.png
   :width: 658px
   :align: center

  Notice that as we increase the number of humanoids (which increases the number of potential contacts in a scene), MJX
  performance degrades more rapidly than MuJoCo.  At the limit, for such a large scene, MuJoCo performance nearly
  matches MJX.

Scenes with collisions between meshes with many vertices
  MJX supports mesh geometries and can determine if two meshes are colliding using branchless versions of
  `mesh collision algorithms <https://ubm-twvideo01.s3.amazonaws.com/o1/vault/gdc2013/slides/822403Gregorius_Dirk_TheSeparatingAxisTest.pdf>`__.
  These algorithms work well for smaller meshes (with hundreds of vertices) but suffer with large meshes. With careful
  tuning, MJX can simulate scenes with mesh collisions well -- see the MJX
  `shadow hand <https://github.com/google-deepmind/mujoco/tree/main/mjx/mujoco/mjx/benchmark/model/shadow_hand>`__
  config for an example.

.. _MjxPerformance:

Performance tuning
==================

For MJX to perform well, some configuration parameters should be adjusted from their default MuJoCo values:

:ref:`option` element
  For now, solver must be set to ``CG`` (but Newton is on its way!).  The ``iterations`` and ``ls_iterations``
  attributes---which control solver and linesearch iterations, respectively---should be brought down to just low enough
  that the simulation remains stable.  Accurate solver forces are not so important in reinforcement learning in which
  domain randomization is often used to add noise to physics for sim2real.

:ref:`contact-pair` element
  Consider explicitly marking geoms for collision detection to reduce the number of contacts that MJX must consider
  during each step.  Enabling only an explicit list of valid contacts can have a dramatic effect on simulation
  performance in MJX.  Doing this well often requires an understanding of the task -- for example, the
  `OpenAI Gym Humanoid <https://github.com/openai/gym/blob/master/gym/envs/mujoco/humanoid_v4.py>`__ task resets when
  the humanoid starts to fall, so full contact with the floor is not needed.

:ref:`option-flag` element
  Disabling ``eulerdamp`` can help performance and is often not needed for stability.
