pipeline {
	agent {
		label 'Windows2019'
	}

	options {
		disableConcurrentBuilds()
		skipDefaultCheckout(true)
	}

	environment {
		SML_BRANCH = 'master'
	}

	stages {
		stage('Checkout') {
			steps {
				checkout scm: [
					$class: 'GitSCM',
					branches: scm.branches,
					extensions: [[
						$class: 'RelativeTargetDirectory',
						relativeTargetDir: 'FicsIt-Networks'
					]],
					submoduleCfg: scm.submoduleCfg,
					doGenerateSubmoduleConfigurations: scm.doGenerateSubmoduleConfigurations,
					userRemoteConfigs: scm.userRemoteConfigs
				]
			}
		}

		stage('Setup UE4') {
			steps {
				dir('ue4') {
					withCredentials([string(credentialsId: 'GitHub-API', variable: 'GITHUB_TOKEN')]) {
						retry(3) {
							bat label: '', script: 'github-release download --user SatisfactoryModdingUE --repo UnrealEngine -l -n "UnrealEngine-CSS-Editor-Win64.zip" > UnrealEngine-CSS-Editor-Win64.zip'
						}
						bat label: '', script: '7z x UnrealEngine-CSS-Editor-Win64.zip'
					}
					bat label: '', script: 'SetupScripts\\Register.bat'
				}
			}
		}
		
		stage('Setup WWise & Rider') {
			steps {
				dir("FicsIt-Networks") {
					bat label: '', script: '7z x %WWISE_PLUGIN% -oPlugins\\'
				}
			}
		}
		
		stage('Setup SML & Assets') {
			steps {
				bat label: '', script: 'git clone --branch %SML_BRANCH% https://github.com/satisfactorymodding/SatisfactoryModLoader.git'
				bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Source\\FactoryGame FicsIt-Networks\\Source\\FactoryGame > copy.log'
				bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Source\\SML FicsIt-Networks\\Source\\SML > copy.log'
				bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Plugins\\Alpakit FicsIt-Networks\\Plugins\\Alpakit > copy.log'
				bat label: '', script: 'xcopy /Y /E /I SatisfactoryModLoader\\Content\\SML FicsIt-Networks\\Content\\SML > copy.log'
				bat label: '', script: '7z x %ASSETS% -oContent\\'
			}
		}

		stage('Build FicsIt-Networks') {
			steps {
				bat label: '', script: '.\\ue4\\Engine\\Binaries\\DotNET\\UnrealBuildTool.exe  -projectfiles -project="%WORKSPACE%\\FicsIt-Networks\\FactoryGame.uproject" -game -rocket -progress'
				bat label: '', script: 'MSBuild.exe .\\FicsIt-Networks\\FactoryGame.sln /p:Configuration="Shipping" /p:Platform="Win64" /t:"Games\\FactoryGame"'
				bat label: '', script: 'MSBuild.exe .\\FicsIt-Networks\\FactoryGame.sln /p:Configuration="Development Editor" /p:Platform="Win64" /t:"Games\\FactoryGame"'
				retry(3) {
					bat label: '', script: '.\\ue4\\Engine\\Build\\BatchFiles\\RunUAT.bat BuildCookRun -nop4 -project="%WORKSPACE%\\FicsIt-Networks\\FactoryGame.uproject" -cook -skipstage -iterate'
				}
				
				dir('ToPak/FactoryGame/Content') {
					bat label: '', script: 'xcopy /Y /E /I %WORKSPACE%\\FicsIt-Networks\\Saved\\Cooked\\WindowsNoEditor\\FactoryGame\\Content\\FicsItNetworks .\\FicsItNetworks > copy.log'
				}
				dir('ToPak') {
					bat label: '', script: 'copy %U4PAK% .'
					bat label: '', script: 'python .\\u4pak.py pack FicsItNetworks.pak FactoryGame'
					bat label: '', script: 'rmdir /S /Q .\\FactoryGame'
				}
				dir('ToPak/FactoryGame/Content') {
					bat label: '', script: 'xcopy /Y /E /I %WORKSPACE%\\FicsIt-Networks\\Saved\\Cooked\\WindowsNoEditor\\FactoryGame\\Content\\SML .\\SML > copy.log'
				}
				dir('ToPak') {
					bat label: '', script: 'copy %U4PAK% .'
					bat label: '', script: 'python .\\u4pak.py pack SML.pak FactoryGame'
				}
			}
		}

		stage('Package FicsIt-Networks') {
			when {
				not {
					changeRequest()
				}
			}
			steps {
				dir('FicsIt-Networks') {
					bat label: '', script: "7z a -tzip -mx9 -mm=LZMA -xr!*.pdb ..\\FicsIt-Networks-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip Binaries\\ Config\\ Content\\ Library\\ Plugins\\Alpakit\\ Source\\ .gitattributes .gitignore FactoryGame.uproject LICENSE README.md ..\\ToPak"
					bat label: '', script: 'copy .\\Binaries\\Win64\\UE4-FicsItNetworks-Win64-Shipping.* .\\'
					bat label: '', script: 'copy ..\\ToPak\\FicsItNetworks.pak .\\'
					bat label: '', script: "7z a -tzip -mx9 FicsIt-Networks.smod .\\data.json .\\FicsItNetworks.png .\\UE4-FicsItNetworks-Win64-Shipping.dll .\\UE4-FicsItNetworks-Win64-Shipping.pdb .\\FicsItNetworks.pak"
				}
				bat label: '', script: '7z x %EXTRACT_AND_GO% -o.\\'
				bat label: '', script: 'copy .\\FicsIt-Networks\\Binaries\\Win64\\UE4-SML-Win64-Shipping.* .\\ExtractAndGo\\loaders\\'
				bat label: '', script: 'copy .\\ToPak\\SML.pak .\\ExtractAndGo\\loaders\\'
				bat label: '', script: 'copy .\\FicsIt-Networks\\Binaries\\Win64\\UE4-FicsItNetworks-Win64-Shipping.* .\\ExtractAndGo\\mods\\'
				bat label: '', script: 'copy .\\ToPak\\FicsItNetworks.pak .\\ExtractAndGo\\mods\\'
				bat label: '', script: "7z a -tzip -mx9 -mm=LZMA FicsIt-Networks-ExtractAndGo-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip .\\ExtractAndGo\\*"
				archiveArtifacts artifacts: "FicsIt-Networks-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip", fingerprint: true, onlyIfSuccessful: true
				archiveArtifacts artifacts: "FicsIt-Networks-ExtractAndGo-${BRANCH_NAME}-${BUILD_NUMBER}-Win64.zip", fingerprint: true, onlyIfSuccessful: true
				archiveArtifacts artifacts: 'FicsIt-Networks\\Binaries\\Win64\\UE4-FicsItNetworks-Win64-Shipping.dll', fingerprint: true, onlyIfSuccessful: true
				archiveArtifacts artifacts: 'ToPak\\FicsItNetworks.pak', fingerprint: true, onlyIfSuccessful: true
				archiveArtifacts artifacts: 'FicsIt-Networks\\FicsIt-Networks.smod', fingerprint: true, onlyIfSuccessful: true
			}
		}
	}
	post {
		always {
			cleanWs()
		}
	}
}
