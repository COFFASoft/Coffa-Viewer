## Please cite this work as: Coffa-ANR-GSCOP-Grenoble 😊
Give it a star ★
# Coffa-Viewer Description
An Open Source 3D Viewer (Part of COFFA Project from [GSCOP Laboratory](https://g-scop.grenoble-inp.fr/en/research/product-process-design)) based on Opencascade.
![image](https://user-images.githubusercontent.com/64405374/142596169-0f65ed11-093f-4b4b-8367-be227ea559f5.png)

You can clone and customize the CoffaViewer with respect to your context. You can also contribute to improve the tool.
The 3D kernel is based on Opencascade Library and the Front-end is based on Qt.
![Visual](https://user-images.githubusercontent.com/64405374/138902371-82b68e87-4684-47f0-9d13-a69166f43b2c.png)

## Downloads
A visual studio project is provided in the repository. You need to download and install sources for OCCT and Qt as indicated below. 
#### **Download Opencascade > 7.5.0**: https://dev.opencascade.org/release.
#### **Download Qt > 5.12.0**: https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5
## Tools
A set of tools have already been implemented in the software. Some are cited below.
### CAD Exchange
Read and Write CAD files. The supported standard formats are STEP, STL and BREP. The viewer can also handle multiple parts simultaneously.
### Rotate & Move
In CoffaViewer Geometries can be transformed (i.e. rotation & translation) by using interactive tools. The transformations are performed with respect to the global reference (X, Y, Z). The transformed shape can be exported into the same or a different format.
### 3D Printing Tools
Only 3D printer plate is implemented in this version.
### Shape Properties
The tool can display the following properties for a shape:
- Location (bounding box center)
- Total surface area in mm^2
- Total volume in mm^3
- Dimensions based on bounding box
- Number of faces

## Possible Improvements for 3D Printing Purposes
- Overhanging Detection
- Support Structures Generation
- Slicing & Tool Path Generation

## LICENSE
Coffa Viewer is General Public Licensed (v3.0), as found in [LICENSE](LICENSE) file
## More Information
(LinkedIn: https://www.linkedin.com/in/mouhamadou-mansour-mbow-bb4b7684/)
(Google Scholar: https://scholar.google.com/citations?user=Ehyud-MAAAAJ&hl=fr)
