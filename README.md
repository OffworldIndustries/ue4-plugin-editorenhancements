# OWI_UE4Tools

Documentation can also be found here:
https://offworldindustries.atlassian.net/wiki/spaces/TR/pages/1119748156/Editor+Tools

## Installation

Download the latest release from here and unzip it into the plugins folder of your project.
You may have to adjust the Unreal Engine version in the OWI_EditorEnhancements.uplugin file.

## Usage

In your project go to Edit -> Project Settings -> OWI Naming Conventions / OWI Reference Rules / OWI Texture Importer and edit the settings there.
To get started you may use the DefaultEditor.ini file from this project / the zip file (copy the contents into your DefaultEditor.ini).

Now everytime an Asset is saved it's naming convention and references will be checked and Unreal will complain in case there are any issues.
To manually trigger it right-click a file/folder and select "Validate Asset/Folder".

Also when importing a Texture (and at least 1 Preset exists) a dialog will appear to change the settings for the imported texture.

## Help

In case you have any questions please feel free to reach out to Jonas Roth :-)
