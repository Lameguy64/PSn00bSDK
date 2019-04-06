# This plugin is part of Scarlet Engine (formerly Project Scarlet)
#
# It is still a work in progress and the SMX file specification may change
# in future versions.

"""
This script exports Scarlet Game Engine SDK compatible SMX files.
Supports normals, colors and texture mapped triangles.
Only one object can be exported at a time.
"""

import os
import bpy

from bpy.props import (CollectionProperty,
                       StringProperty,
                       BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       )

from bpy_extras.io_utils import (ImportHelper,
                                 ExportHelper,
                                 axis_conversion,
                                 )

bl_info = {
    "name":         "Export: Project Scarlet SMX Raw Model",
    "author":       "Jobert 'Lameguy' Villamor (Lameguy64)",
    "blender":      (2,6,9),
    "version":      (3,1,2),
    "location":     "File > Export",
    "description":  "Export mesh to Project Scarlet SMX model format",
    "category":     "Import-Export"
}

class ExportSMX(bpy.types.Operator, ExportHelper):
    
    bl_idname       = "export_test.smx";
    bl_label        = "Export SMX";
    
    filename_ext    = ".smx";
    filter_glob		= StringProperty(default="*.smx", options={'HIDDEN'})
    
    # Export options
    exp_applyModifiers = BoolProperty(
        name="Apply Modifiers",
        description="Apply modifiers to the exported mesh",
        default=True,
        )
    
    exp_writeNormals = BoolProperty(
        name="Normals",
        description="Export normals for smooth and hard shaded faces",
        default=True,
        )
    
    #exp_vertexWeights = BoolProperty(
    #    name="Vertex Weights",
    #    description="Export vertex weights",
    #    default=False,
    #    )
    
    #exp_vertexGroups = BoolProperty(
    #    name="Vertex Groups",
    #    description="Export vertex group information",
    #    default=False,
    #    )
    
    #exp_doubleSided = BoolProperty(
    #    name="Double-sided",
    #    description="Sets the double-sided attribute to all exported polygons",
    #    default=False,
    #    )
        
    #exp_subsurfMode = BoolProperty(
    #    name="Subsurf-Mode",
    #    description="Enable this if the mesh you're exporting uses a subsurf divide modifier, faster but often wasteful",
    #    default=False,
    #    )
        
    #exp_blendMode = EnumProperty(
    #    name="Semi-trans:",
    #    description="Sets the semi-transparency attribute for all exported polygons",
    #    items=(('BN', "Off", ""),
    #           ('B0', "0: 50%B + 50%F", ""),
    #           ('B1', "1: 100%B + 100%F", ""),
    #           ('B2', "2: 100%B - 100%F", ""),
    #           ('B3', "3: 100%B + 25%F", ""),
    #           ),
    #    default='BN',
    #    )
    
    #exp_scaleFactor = FloatProperty(
    #    name="Scale Factor",
    #    description="Scale factor of exported mesh",
    #    min=0.01, max=1000.0,
    #    default=1.0,
    #    )
        
    def execute(self, context):

        print("Export execute...\n")
        
        obj = context.object
        mesh = obj.to_mesh(context.scene, self.exp_applyModifiers, 'PREVIEW')
        
        if not mesh.tessfaces and mesh.polygons:
            mesh.calc_tessface()
        
        filepath = self.filepath
        filepath = bpy.path.ensure_ext(filepath, self.filename_ext)
        
        with open(filepath, "w") as f:
            
            # Write a banner
            f.write("<!-- Created using Project Scarlet SMX Export Plug-in for Blender -->\n")
            f.write("<!-- NOTE: If you plan to use this model as a static mesh, it is recommended that you run this file through smxopt -->\n")
            f.write("<!-- or smxtool to clean up duplicate/unused normals which are kept for animation purposes. -->\n")
            
            f.write("<model version=\"1\">\n")
            
            # Write vertices
            f.write("<vertices count=\"%d\">\n" % len(mesh.vertices))
            for v in mesh.vertices:
                f.write("<v x=\"%f\" y=\"%f\" z=\"%f\"/>\n" % (v.co.x, -v.co.z, v.co.y))
            f.write("</vertices>\n")
            
            
            # Scan if there are any flat primitives
            has_flats = False
            for i,p in enumerate(mesh.tessfaces):
                if p.use_smooth is False:
                    has_flats = True
                    break

            # Export normals
            if self.exp_writeNormals:
                if has_flats:
                    f.write("<normals count=\"%d\">\n" % (len(mesh.vertices)+len(mesh.polygons)))
                else:
                    f.write("<normals count=\"%d\">\n" % (len(mesh.vertices)))
                f.write("<!-- Smooth normals begin here -->\n")
                for v in mesh.vertices:
                    f.write("<v x=\"%f\" y=\"%f\" z=\"%f\"/>\n" % (v.normal.x, -v.normal.z, v.normal.y))
                if has_flats:
                    f.write("<!-- Flat normals begin here -->\n")
                    flatnorms_start = len(mesh.vertices)
                    for p in mesh.polygons:
                        f.write("<v x=\"%f\" y=\"%f\" z=\"%f\"/>\n" % (p.normal.x, -p.normal.z, p.normal.y))
                f.write("</normals>\n")


            # Write texture files
            mesh_uvs = mesh.tessface_uv_textures.active
                
            if mesh_uvs is not None:
                mesh_uvs = mesh_uvs.data
            
            # Scan through all faces for assigned textures
            if mesh_uvs is not None:
                tex_table = []
                tex_files = []
                for uv in mesh_uvs:
                    if uv.image is not None:
                        addTex = True
                        texFileName = bpy.path.display_name_from_filepath(uv.image.filepath)
                        if len(tex_files)>0:
                            for c,t in enumerate(tex_files):
                                if t == texFileName:
                                    tex_table.append(c+1)
                                    addTex = False
                                    break
                        if addTex:
                            print("TF:%s" % (texFileName))
                            tex_files.append(texFileName)
                            tex_table.append(len(tex_files))   
                    else:
                        tex_table.append(0)
                        
                # Write texture files
                f.write("<textures count=\"%d\">\n" % len(tex_files))
                for n in tex_files:
                    f.write("<texture file=\"%s\"/>\n" % n)
                f.write("</textures>\n")
            else:
                tex_table = None
                tex_files = None        
            
            
            mesh_cols = mesh.tessface_vertex_colors.active
                
            if mesh_cols is not None:
                mesh_cols = mesh_cols.data
                    
            tri_indices = ( 0, 2, 1 );
            quad_indices = ( 3, 2, 0, 1 );
            
            f.write("<primitives count=\"%d\">\n" % len(mesh.tessfaces))
            for i,p in enumerate(mesh.tessfaces):
                
                # Write vertex indices
                f.write("<poly ")
                if (len(p.vertices) == 3):
                    f.write("v0=\"%d\" " % (p.vertices[0]))
                    f.write("v1=\"%d\" " % (p.vertices[2]))
                    f.write("v2=\"%d\" " % (p.vertices[1]))
                elif (len(p.vertices) == 4):
                    f.write("v0=\"%d\" " % (p.vertices[3]))
                    f.write("v1=\"%d\" " % (p.vertices[2]))
                    f.write("v2=\"%d\" " % (p.vertices[0]))
                    f.write("v3=\"%d\" " % (p.vertices[1]))
                
                # Write normal indices and shading mode
                if self.exp_writeNormals:
                    if p.use_smooth:
                        if (len(p.vertices) == 3):
                            f.write("n0=\"%d\" " % (p.vertices[0]))
                            f.write("n1=\"%d\" " % (p.vertices[2]))
                            f.write("n2=\"%d\" " % (p.vertices[1]))
                        elif (len(p.vertices) == 4):
                            f.write("n0=\"%d\" " % (p.vertices[3]))
                            f.write("n1=\"%d\" " % (p.vertices[2]))
                            f.write("n2=\"%d\" " % (p.vertices[0]))
                            f.write("n3=\"%d\" " % (p.vertices[1]))
                        f.write("shading=\"S\" ")
                    else:
                        f.write("n0=\"%d\" " % (flatnorms_start+i))
                        f.write("shading=\"F\" ")
                    
                if tex_table is not None:
                    if (tex_table[i] > 0):
                        color_mul = 128.0
                    else:
                        color_mul = 255.0
                else:
                    color_mul = 255.0
					
                # Write out vertex colors if available
                if mesh_cols is None:
                    f.write("r0=\"128\" g0=\"128\" b0=\"128\" ")
                    typecode = "F"
                else:
                    col = mesh_cols[i]
                    col = col.color1[:], col.color2[:], col.color3[:], col.color4[:]
                    # Check if polygon is flat shaded
                    if (col[0] == col[1]) and (col[1] == col[2]) and (col[2] == col[0]):
                        # is flat...
                        color = col[0]
                        color = (int(color[0]*color_mul),
                                 int(color[1]*color_mul),
                                 int(color[2]*color_mul),
                                 )
                        f.write("r0=\"%d\" g0=\"%d\" b0=\"%d\" " % color[:])
                        typecode = "F"
                    else:
                        # is gouraud...
                        for j,c in enumerate(p.vertices):
                            if (len(p.vertices) == 4):
                                color = col[quad_indices[j]]
                            else:
                                color = col[tri_indices[j]]
                            color = (int(color[0]*color_mul),
                                     int(color[1]*color_mul),
                                     int(color[2]*color_mul),
                                     )
                            f.write("r%d=\"%d\" g%d=\"%d\" b%d=\"%d\" " % 
                                (j, color[0], j, color[1], j, color[2]))
                        typecode = "G"
                        
                # Add texcoords
                if tex_table is not None:
                    if (tex_table[i] > 0):
                        f.write("texture=\"%d\" " % (tex_table[i]-1));
                        if (len(p.vertices) == 3):
                            uv = (mesh_uvs[i].uv1, 
                                  mesh_uvs[i].uv3, 
                                  mesh_uvs[i].uv2
                                  )
                        elif (len(p.vertices) == 4):
                            uv = (mesh_uvs[i].uv4, 
                                  mesh_uvs[i].uv3, 
                                  mesh_uvs[i].uv1, 
                                  mesh_uvs[i].uv2
                                  )
                        tex_w = mesh_uvs[i].image.size[0]-0.85#(1.0/mesh_uvs[i].image.size[0])
                        tex_h = mesh_uvs[i].image.size[1]-0.85#(1.0-(1.0/mesh_uvs[i].image.size[1]))
                        for j,c in enumerate(uv):
                            f.write("tu%d=\"%d\" tv%d=\"%d\" " % 
                                (j, round(tex_w*uv[j].x), j, round(tex_h-(tex_h*uv[j].y))))
                        typecode += "T"
                    
                typecode += "%d" % len(p.vertices)
                f.write("type=\"%s\" " % typecode)
                f.write("/>\n")

            f.write("</primitives>\n")
            
            f.write("</model>")
            
            f.close()
            
        return {'FINISHED'};
    
# For registering to Blender menus
def menu_func(self, context):
    self.layout.operator(ExportSMX.bl_idname, text="Scarlet 3D SMX v3 (.smx)");

def register():
    bpy.utils.register_module(__name__);
    bpy.types.INFO_MT_file_export.append(menu_func);
    
def unregister():
    bpy.utils.unregister_module(__name__);
    bpy.types.INFO_MT_file_export.remove(menu_func);

if __name__ == "__main__":
    register()
    # Uncomment when testing this script
    #bpy.ops.export_test.smx('INVOKE_DEFAULT')