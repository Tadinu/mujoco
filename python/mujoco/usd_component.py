import numpy as np
import open3d as o3d
from usd_utils import *
from mujoco import mjtGeom
from pxr import Gf, Sdf, Vt
from typing import Optional, List
from mujoco import _structs, _enums
from pxr import Usd, UsdGeom, UsdShade, UsdLux

class USDMesh:

    def __init__(
        self,
        stage: Usd.Stage,
        model: _structs.MjModel,
        geom: _structs.MjvGeom,
        objid: int,
        dataid: int,
        rgba: List[int] = [1,1,1,1],
        texture_file: Optional[str] = None
    ):
        """ Initializes a new USD mesh
        Args:
            model: an MjModel instance.
            dataid: id of the mesh
            texture_file: texture associated with the mesh
        """
        self.stage = stage
        self.model = model
        self.geom = geom
        self.objid = objid
        self.rgba = rgba
        self.dataid = dataid
        self.texture_file = texture_file

        xform_path = f'/World/Mesh_Xform_{objid}'
        mesh_path= f'{xform_path}/Mesh_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_mesh = UsdGeom.Mesh.Define(stage, mesh_path)
        self.usd_prim = stage.GetPrimAtPath(mesh_path)

        # setting mesh structure properties
        mesh_vert, mesh_face, mesh_facenum = self._get_mesh_geometry()
        self.usd_mesh.GetPointsAttr().Set(mesh_vert)
        self.usd_mesh.GetFaceVertexCountsAttr().Set([3 for _ in range(mesh_facenum)])
        self.usd_mesh.GetFaceVertexIndicesAttr().Set(mesh_face)

        # setting mesh uv properties
        mesh_texcoord, mesh_facetexcoord = self._get_uv_geometry()
        self.texcoords = UsdGeom.PrimvarsAPI(self.usd_mesh).CreatePrimvar("UVMap",
                                        Sdf.ValueTypeNames.TexCoord2fArray,
                                        UsdGeom.Tokens.faceVarying)
        self.texcoords.Set(mesh_texcoord)
        self.texcoords.SetIndices(Vt.IntArray(mesh_facetexcoord.tolist()))

        self._attach_material()
        
        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()

    def get_facetexcoord_ranges(self, nmesh, arr):
        facetexcoords_ranges = [0]
        running_sum = 0
        for i in range(nmesh):
            running_sum += arr[i] * 3
            facetexcoords_ranges.append(running_sum)
        return facetexcoords_ranges

    def _get_uv_geometry(self):
        mesh_texcoord_adr_from = self.model.mesh_texcoordadr[self.dataid]
        mesh_texcoord_adr_to = self.model.mesh_texcoordadr[self.dataid+1] if self.dataid < self.model.nmesh - 1 else len(self.model.mesh_texcoord)
        mesh_texcoord = self.model.mesh_texcoord[mesh_texcoord_adr_from:mesh_texcoord_adr_to]

        mesh_facetexcoord_ranges = self.get_facetexcoord_ranges(self.model.nmesh, self.model.mesh_facenum)

        mesh_facetexcoord = self.model.mesh_facetexcoord.flatten()
        mesh_facetexcoord = mesh_facetexcoord[mesh_facetexcoord_ranges[self.dataid]:mesh_facetexcoord_ranges[self.dataid+1]]
    
        return mesh_texcoord, mesh_facetexcoord

    def _get_mesh_geometry(self):
        # get mesh geometry structure from reading the mjModel
        mesh_vert_adr_from = self.model.mesh_vertadr[self.dataid]
        mesh_vert_adr_to = self.model.mesh_vertadr[self.dataid+1] if self.dataid < self.model.nmesh - 1 else len(self.model.mesh_vert)
        mesh_vert = self.model.mesh_vert[mesh_vert_adr_from:mesh_vert_adr_to]

        mesh_face_adr_from = self.model.mesh_faceadr[self.dataid]
        mesh_face_adr_to = self.model.mesh_faceadr[self.dataid+1] if self.dataid < self.model.nmesh - 1 else len(self.model.mesh_face)
        mesh_face = self.model.mesh_face[mesh_face_adr_from:mesh_face_adr_to]

        mesh_facenum = self.model.mesh_facenum[self.dataid]

        return mesh_vert, mesh_face, mesh_facenum
    
    def _attach_material(self):
        mtl_path = Sdf.Path(f"/World/_materials/Material_{self.objid}")
        mtl = UsdShade.Material.Define(self.stage, mtl_path)

        if self.texture_file:
            bsdf_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Principled_BSDF"))
            image_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Image_Texture"))
            uvmap_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("uvmap"))

            # setting the bsdf shader attributes
            bsdf_shader.CreateIdAttr("UsdPreviewSurface")
            bsdf_shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(image_shader.ConnectableAPI(), "rgb")
            bsdf_shader.CreateInput("opacity", Sdf.ValueTypeNames.Float).Set(float(self.rgba[-1]))
            bsdf_shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.5)
            bsdf_shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

            mtl.CreateSurfaceOutput().ConnectToSource(bsdf_shader.ConnectableAPI(), "surface")

            self.usd_mesh.GetPrim().ApplyAPI(UsdShade.MaterialBindingAPI)
            UsdShade.MaterialBindingAPI(self.usd_mesh).Bind(mtl)

            # setting the image texture attributes
            image_shader.CreateIdAttr("UsdUVTexture")
            image_shader.CreateInput("file", Sdf.ValueTypeNames.Asset).Set(self.texture_file)
            image_shader.CreateInput("sourceColorSpace", Sdf.ValueTypeNames.Token).Set("sRGB")
            image_shader.CreateInput("wrapS", Sdf.ValueTypeNames.Token).Set("repeat")
            image_shader.CreateInput("wrapT", Sdf.ValueTypeNames.Token).Set("repeat")
            image_shader.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(uvmap_shader.ConnectableAPI(), "result")
            image_shader.CreateOutput('rgb', Sdf.ValueTypeNames.Float3)

            # setting uvmap shader attributes
            uvmap_shader.CreateIdAttr("UsdPrimvarReader_float2")
            uvmap_shader.CreateInput("varname", Sdf.ValueTypeNames.Token).Set("UVMap")
            uvmap_shader.CreateOutput('results', Sdf.ValueTypeNames.Float2)
        else:
            bsdf_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Principled_BSDF"))

            # settings the bsdf shader attributes
            bsdf_shader.CreateIdAttr("UsdPreviewSurface")
            bsdf_shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(tuple(self.rgba[:3]))
            bsdf_shader.CreateInput("opacity", Sdf.ValueTypeNames.Float).Set(float(self.rgba[-1]))
            bsdf_shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.5)
            bsdf_shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

        mtl.CreateSurfaceOutput().ConnectToSource(bsdf_shader.ConnectableAPI(), "surface")

        self.usd_mesh.GetPrim().ApplyAPI(UsdShade.MaterialBindingAPI)
        UsdShade.MaterialBindingAPI(self.usd_mesh).Bind(mtl)

    def update(
        self,
        pos: np.array,
        mat: np.array,
        frame: int
    ):
        transformation_mat = create_transform_matrix(rotation_matrix=mat, translation_vector=pos).T
        self.transform_op.Set(Gf.Matrix4d(transformation_mat.tolist()), frame)

class USDPrimitiveMesh:

    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        self.stage = stage
        self.geom = geom
        self.objid = objid
        self.rgba = rgba
        self.texture_file = texture_file

        self.prim_mesh = None

    def _get_uv_geometry(self):    

        assert self.prim_mesh

        x_scale, y_scale = self.geom.texrepeat

        mesh_texcoord = np.array(self.prim_mesh.triangle_uvs)
        mesh_facetexcoord = np.asarray(self.prim_mesh.triangles)

        mesh_texcoord[:, 0] *= x_scale
        mesh_texcoord[:, 1] *= y_scale

        return mesh_texcoord, mesh_facetexcoord.flatten()

    def _get_mesh_geometry(self):

        assert self.prim_mesh

        # get mesh geometry from the open3d mesh model
        mesh_vert = np.asarray(self.prim_mesh.vertices)
        mesh_face = np.asarray(self.prim_mesh.triangles)

        return mesh_vert, mesh_face, len(mesh_face)

    def _attach_material(self):
        mtl_path = Sdf.Path(f"/World/_materials/Material_{self.objid}")
        mtl = UsdShade.Material.Define(self.stage, mtl_path)
        if self.texture_file:
            # remove all code in this if block
            bsdf_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Principled_BSDF"))
            image_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Image_Texture"))
            uvmap_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("uvmap"))

            # setting the bsdf shader attributes
            bsdf_shader.CreateIdAttr("UsdPreviewSurface")
            bsdf_shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(image_shader.ConnectableAPI(), "rgb")
            bsdf_shader.CreateInput("opacity", Sdf.ValueTypeNames.Float).Set(float(self.rgba[-1]))
            bsdf_shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.5)
            bsdf_shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

            mtl.CreateSurfaceOutput().ConnectToSource(bsdf_shader.ConnectableAPI(), "surface")

            self.usd_mesh.GetPrim().ApplyAPI(UsdShade.MaterialBindingAPI)
            UsdShade.MaterialBindingAPI(self.usd_mesh).Bind(mtl)

            # setting the image texture attributes
            image_shader.CreateIdAttr("UsdUVTexture")
            image_shader.CreateInput("file", Sdf.ValueTypeNames.Asset).Set(self.texture_file)
            image_shader.CreateInput("sourceColorSpace", Sdf.ValueTypeNames.Token).Set("sRGB")
            image_shader.CreateInput("wrapS", Sdf.ValueTypeNames.Token).Set("repeat")
            image_shader.CreateInput("wrapT", Sdf.ValueTypeNames.Token).Set("repeat")
            image_shader.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(uvmap_shader.ConnectableAPI(), "result")
            image_shader.CreateOutput('rgb', Sdf.ValueTypeNames.Float3)

            # setting uvmap shader attributes
            uvmap_shader.CreateIdAttr("UsdPrimvarReader_float2")
            uvmap_shader.CreateInput("varname", Sdf.ValueTypeNames.Token).Set("UVMap")
            uvmap_shader.CreateOutput('results', Sdf.ValueTypeNames.Float2)
        else:
            bsdf_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Principled_BSDF"))

            # settings the bsdf shader attributes
            bsdf_shader.CreateIdAttr("UsdPreviewSurface")
            bsdf_shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(tuple(self.rgba[:3]))
            bsdf_shader.CreateInput("opacity", Sdf.ValueTypeNames.Float).Set(float(self.rgba[-1]))
            bsdf_shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.5)
            bsdf_shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

        mtl.CreateSurfaceOutput().ConnectToSource(bsdf_shader.ConnectableAPI(), "surface")

        self.usd_mesh.GetPrim().ApplyAPI(UsdShade.MaterialBindingAPI)
        UsdShade.MaterialBindingAPI(self.usd_mesh).Bind(mtl)

    def update(
        self,
        pos: np.array,
        mat: np.array,
        frame: int
    ):
        transformation_mat = create_transform_matrix(rotation_matrix=mat, translation_vector=pos).T
        self.transform_op.Set(Gf.Matrix4d(transformation_mat.tolist()), frame)

class USDPrimitive:
    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        self.stage = stage
        self.geom = geom
        self.objid = objid
        self.rgba = rgba
        self.texture_file = texture_file

    def _attach_material(self):
        mtl_path = Sdf.Path(f"/World/_materials/Material_{self.objid}")
        mtl = UsdShade.Material.Define(self.stage, mtl_path)
        if self.texture_file:
            bsdf_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Principled_BSDF"))
            image_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Image_Texture"))
            uvmap_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("uvmap"))

            # settings the bsdf shader attributes
            bsdf_shader.CreateIdAttr("UsdPreviewSurface")
            bsdf_shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(image_shader.ConnectableAPI(), "rgb")
            bsdf_shader.CreateInput("opacity", Sdf.ValueTypeNames.Float).Set(float(self.rgba[-1]))
            bsdf_shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.5)
            bsdf_shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

            mtl.CreateSurfaceOutput().ConnectToSource(bsdf_shader.ConnectableAPI(), "surface")

            self.usd_primitive_shape.GetPrim().ApplyAPI(UsdShade.MaterialBindingAPI)
            UsdShade.MaterialBindingAPI(self.usd_primitive_shape).Bind(mtl)

            # setting the image texture attributes
            image_shader.CreateIdAttr("UsdUVTexture")
            image_shader.CreateInput("file", Sdf.ValueTypeNames.Asset).Set(self.texture_file)
            image_shader.CreateInput("sourceColorSpace", Sdf.ValueTypeNames.Token).Set("sRGB")
            image_shader.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(uvmap_shader.ConnectableAPI(), "result")
            image_shader.CreateOutput('rgb', Sdf.ValueTypeNames.Float3)

            # setting uvmap shader attributes
            uvmap_shader.CreateIdAttr("UsdPrimvarReader_float2")
            uvmap_shader.CreateInput("varname", Sdf.ValueTypeNames.Token).Set("UVMap")
            uvmap_shader.CreateOutput('results', Sdf.ValueTypeNames.Float2)
        else:
            bsdf_shader = UsdShade.Shader.Define(self.stage, mtl_path.AppendPath("Principled_BSDF"))

            # settings the bsdf shader attributes
            bsdf_shader.CreateIdAttr("UsdPreviewSurface")
            bsdf_shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(tuple(self.rgba[:3]))
            bsdf_shader.CreateInput("opacity", Sdf.ValueTypeNames.Float).Set(float(self.rgba[-1]))
            bsdf_shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).Set(0.5)
            bsdf_shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).Set(0.0)

        mtl.CreateSurfaceOutput().ConnectToSource(bsdf_shader.ConnectableAPI(), "surface")

        self.usd_primitive_shape.GetPrim().ApplyAPI(UsdShade.MaterialBindingAPI)
        UsdShade.MaterialBindingAPI(self.usd_primitive_shape).Bind(mtl)

    def update(
        self,
        pos: np.array,
        mat: np.array,
        frame: int
    ):
        transformation_mat = create_transform_matrix(rotation_matrix=mat, translation_vector=pos).T
        self.transform_op.Set(Gf.Matrix4d(transformation_mat.tolist()), frame)

class USDCapsule(USDPrimitive):
    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        
        super().__init__(stage,
                         geom,
                         objid,
                         rgba,
                         texture_file)
        
        xform_path = f'/World/Capsule_Xform_{objid}'
        capsule_path = f'{xform_path}/Capsule_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_primitive_shape = UsdGeom.Capsule.Define(stage, capsule_path)
        self.usd_prim = stage.GetPrimAtPath(capsule_path)
        
        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()
        self.scale_op = self.usd_xform.AddScaleOp()

        # setting attributes for the shape
        self._set_size_attributes()
        self._attach_material()

    def _set_size_attributes(self):        
        self.usd_primitive_shape.GetRadiusAttr().Set(float(self.geom.size[0]))
        self.usd_primitive_shape.GetHeightAttr().Set(float(self.geom.size[2]*2)) # mujoco gives the half length

class USDEllipsoid(USDPrimitive):
    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        
        super().__init__(stage,
                         geom,
                         objid,
                         rgba,
                         texture_file)
        
        xform_path = f'/World/Ellipsoid_Xform_{objid}'
        ellipsoid_path = f'{xform_path}/Ellipsoid_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_primitive_shape = UsdGeom.Sphere.Define(stage, ellipsoid_path)
        self.usd_prim = stage.GetPrimAtPath(ellipsoid_path)

        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()
        self.scale_op = self.usd_xform.AddScaleOp()

        # setting attributes for the shape
        self._set_size_attributes()
        self._attach_material()

    def _set_size_attributes(self):        
        self.scale_op.Set(Gf.Vec3d(self.geom.size.tolist()))

class USDCubeMesh(USDPrimitiveMesh):
    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        
        super().__init__(stage,
                         geom,
                         objid,
                         rgba,
                         texture_file)

        xform_path = f'/World/CubeMesh_Xform_{objid}'
        mesh_path= f'{xform_path}/CubeMesh_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_mesh = UsdGeom.Mesh.Define(stage, mesh_path)
        self.usd_prim = stage.GetPrimAtPath(mesh_path)

        self.prim_mesh = o3d.geometry.TriangleMesh.create_box(width=self.geom.size[0]*2, 
                                                              height=self.geom.size[1]*2,
                                                              depth=self.geom.size[2]*2,
                                                              create_uv_map=True,
                                                              map_texture_to_each_face=True)

        self.prim_mesh.translate(-self.prim_mesh.get_center())

        mesh_vert, mesh_face, mesh_facenum = self._get_mesh_geometry()
        self.usd_mesh.GetPointsAttr().Set(mesh_vert)
        self.usd_mesh.GetFaceVertexCountsAttr().Set([3 for _ in range(mesh_facenum)])
        self.usd_mesh.GetFaceVertexIndicesAttr().Set(mesh_face)

        # setting mesh uv properties
        mesh_texcoord, mesh_facetexcoord = self._get_uv_geometry()
        self.texcoords = UsdGeom.PrimvarsAPI(self.usd_mesh).CreatePrimvar("UVMap",
                                        Sdf.ValueTypeNames.TexCoord2fArray,
                                        UsdGeom.Tokens.faceVarying)
                
        self.texcoords.Set(mesh_texcoord)
        self.texcoords.SetIndices(Vt.IntArray([i for i in range(mesh_facenum*3)]))

        # setting attributes for the shape
        self._attach_material()

        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()

class USDSphereMesh(USDPrimitiveMesh):
    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        
        super().__init__(stage,
                         geom,
                         objid,
                         rgba,
                         texture_file)

        xform_path = f'/World/SphereMesh_Xform_{objid}'
        mesh_path= f'{xform_path}/SphereMesh_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_mesh = UsdGeom.Mesh.Define(stage, mesh_path)
        self.usd_prim = stage.GetPrimAtPath(mesh_path)

        self.prim_mesh = o3d.geometry.TriangleMesh.create_sphere(radius=float(self.geom.size[0]),
                                                                 create_uv_map=True)

        self.prim_mesh.translate(-self.prim_mesh.get_center())

        mesh_vert, mesh_face, mesh_facenum = self._get_mesh_geometry()
        self.usd_mesh.GetPointsAttr().Set(mesh_vert)
        self.usd_mesh.GetFaceVertexCountsAttr().Set([3 for _ in range(mesh_facenum)])
        self.usd_mesh.GetFaceVertexIndicesAttr().Set(mesh_face)

        # setting mesh uv properties
        mesh_texcoord, mesh_facetexcoord = self._get_uv_geometry()
        self.texcoords = UsdGeom.PrimvarsAPI(self.usd_mesh).CreatePrimvar("UVMap",
                                        Sdf.ValueTypeNames.TexCoord2fArray,
                                        UsdGeom.Tokens.faceVarying)
                
        self.texcoords.Set(mesh_texcoord)
        self.texcoords.SetIndices(Vt.IntArray([i for i in range(mesh_facenum*3)]))

        # setting attributes for the shape
        self._attach_material()

        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()

class USDCylinderMesh(USDPrimitiveMesh):
    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        
        super().__init__(stage,
                         geom,
                         objid,
                         rgba,
                         texture_file)

        xform_path = f'/World/CylinderMesh_Xform_{objid}'
        mesh_path= f'{xform_path}/CylinderMesh_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_mesh = UsdGeom.Mesh.Define(stage, mesh_path)
        self.usd_prim = stage.GetPrimAtPath(mesh_path)

        self.prim_mesh = o3d.geometry.TriangleMesh.create_cylinder(radius=self.geom.size[0],
                                                                   height=self.geom.size[2]*2,
                                                                   create_uv_map=True)

        self.prim_mesh.translate(-self.prim_mesh.get_center())

        mesh_vert, mesh_face, mesh_facenum = self._get_mesh_geometry()
        self.usd_mesh.GetPointsAttr().Set(mesh_vert)
        self.usd_mesh.GetFaceVertexCountsAttr().Set([3 for _ in range(mesh_facenum)])
        self.usd_mesh.GetFaceVertexIndicesAttr().Set(mesh_face)

        # setting mesh uv properties
        mesh_texcoord, mesh_facetexcoord = self._get_uv_geometry()
        self.texcoords = UsdGeom.PrimvarsAPI(self.usd_mesh).CreatePrimvar("UVMap",
                                        Sdf.ValueTypeNames.TexCoord2fArray,
                                        UsdGeom.Tokens.faceVarying)
                
        self.texcoords.Set(mesh_texcoord)
        self.texcoords.SetIndices(Vt.IntArray([i for i in range(mesh_facenum*3)]))

        # setting attributes for the shape
        self._attach_material()

        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()

class USDPlaneMesh(USDPrimitiveMesh):
    def __init__(self,
                 stage: Usd.Stage,
                 geom: _structs.MjvGeom,
                 objid: int,
                 rgba: List[int] = [1,1,1,1],
                 texture_file: Optional[str] = None):
        
        super().__init__(stage,
                         geom,
                         objid,
                         rgba,
                         texture_file)
        
        xform_path = f'/World/Plane_Xform_{objid}'
        plane_path = f'{xform_path}/PlaneMesh_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_mesh = UsdGeom.Mesh.Define(stage, plane_path)
        self.usd_prim = stage.GetPrimAtPath(plane_path)
        
        self.prim_mesh = o3d.geometry.TriangleMesh.create_box(width=self.geom.size[0]*2 if self.geom.size[0] > 0 else 100, 
                                                              height=self.geom.size[1]*2 if self.geom.size[1] > 0 else 100,
                                                              depth=0.001,
                                                              create_uv_map=True,
                                                              map_texture_to_each_face=True)

        self.prim_mesh.translate(-self.prim_mesh.get_center())

        mesh_vert, mesh_face, mesh_facenum = self._get_mesh_geometry()
        self.usd_mesh.GetPointsAttr().Set(mesh_vert)
        self.usd_mesh.GetFaceVertexCountsAttr().Set([3 for _ in range(mesh_facenum)])
        self.usd_mesh.GetFaceVertexIndicesAttr().Set(mesh_face)

        # setting mesh uv properties
        mesh_texcoord, mesh_facetexcoord = self._get_uv_geometry()
        self.texcoords = UsdGeom.PrimvarsAPI(self.usd_mesh).CreatePrimvar("UVMap",
                                        Sdf.ValueTypeNames.TexCoord2fArray,
                                        UsdGeom.Tokens.faceVarying)
                
        self.texcoords.Set(mesh_texcoord)
        self.texcoords.SetIndices(Vt.IntArray([i for i in range(mesh_facenum*3)]))

        # setting attributes for the shape
        self._attach_material()

        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()

class USDLight:
    def __init__(self,
                 stage: Usd.Stage,
                 objid: int,
                 radius: Optional[float] = 0.7):
        self.stage = stage

        xform_path = f'/World/Light_Xform_{objid}'
        light_path = f'{xform_path}/Light_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_light = UsdLux.SphereLight.Define(stage, light_path)
        self.usd_prim = stage.GetPrimAtPath(light_path)

        # we assume in mujoco that all lights are point lights
        self.usd_light.GetRadiusAttr().Set(radius)
        self.usd_light.GetTreatAsPointAttr().Set(False)
        self.usd_light.GetNormalizeAttr().Set(True)

        # defining ops required by update function
        self.translate_op = self.usd_xform.AddTranslateOp()

    def update(self,
               pos: np.array,
               intensity: int,
               color: np.array,
               frame: int):
        self.translate_op.Set(Gf.Vec3d(pos.tolist()), frame)

        if not np.any(pos):
            intensity = 0

        self.usd_light.GetIntensityAttr().Set(intensity)
        self.usd_light.GetColorAttr().Set(Gf.Vec3d(color.tolist()))

class USDCamera:
    def __init__(self,
                 stage: Usd.Stage,
                 objid: int):
        self.stage = stage

        xform_path = f'/World/Camera_Xform_{objid}'
        camera_path = f'{xform_path}/Camera_{objid}'
        self.usd_xform = UsdGeom.Xform.Define(stage, xform_path)
        self.usd_camera = UsdGeom.Camera.Define(stage, camera_path)
        self.usd_prim = stage.GetPrimAtPath(camera_path)

        # defining ops required by update function
        self.transform_op = self.usd_xform.AddTransformOp()

        self.usd_camera.CreateFocalLengthAttr().Set(18.14756) # default in omniverse
        self.usd_camera.CreateFocusDistanceAttr().Set(400)

        self.usd_camera.GetClippingRangeAttr().Set(Gf.Vec2f(1e-4, 1e6))

    def update(self,
               cam_pos: np.array,
               cam_mat: np.array,
               frame: int):
        
        transformation_mat = create_transform_matrix(rotation_matrix=cam_mat, translation_vector=cam_pos).T
        self.transform_op.Set(Gf.Matrix4d(transformation_mat.tolist()), frame)
        

        